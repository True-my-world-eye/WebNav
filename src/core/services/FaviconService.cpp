// FaviconService.cpp — Favicon 抓取服务实现
// 异步获取网页 favicon 并缓存到本地
// 流程：提取域名 → 检查缓存 → 发起网络请求 → 保存到本地

#include "FaviconService.h"
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QBuffer>
#include <QImageReader>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QCryptographicHash>

#include <QDebug>

FaviconService::FaviconService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_memoryCache(100)  // 内存缓存最多100个
{
}

// ── 缓存管理 ──────────────────────────────────────────────────

// 获取 favicon 缓存目录
// 路径：AppDataLocation/favicons/
QString FaviconService::cacheDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                  + "/favicons/";
    QDir().mkpath(dir);
    return dir;
}

// 从 URL 提取域名
// 例如：https://www.example.com/path → www.example.com
QString FaviconService::extractDomain(const QString &pageUrl)
{
    QUrl url(pageUrl);
    return url.host().toLower();
}

// 构建 favicon URL 列表
// 尝试多个位置，按优先级排列：
// 1. 标准 favicon.ico
// 2. Apple Touch Icon（iOS 设备）
// 3. Google S2 Favicon 服务（兜底）
QStringList FaviconService::buildFaviconUrls(const QString &domain)
{
    return {
        QString("https://%1/favicon.ico").arg(domain),
        QString("https://%1/apple-touch-icon.png").arg(domain),
        QString("https://%1/apple-touch-icon-precomposed.png").arg(domain),
        QString("https://www.google.com/s2/favicons?domain=%1&sz=64").arg(domain),
    };
}

// 生成缓存文件名
// 使用域名的 MD5 哈希作为文件名，避免特殊字符问题
QString FaviconService::cacheFileName(const QString &domain)
{
    QByteArray hash = QCryptographicHash::hash(domain.toUtf8(), QCryptographicHash::Md5);
    return cacheDirectory() + hash.toHex() + ".png";
}

// 从缓存中同步获取已缓存的 favicon 路径
// 优先检查内存缓存，再检查本地文件
QString FaviconService::getCachedFavicon(const QString &pageUrl) const
{
    QString domain = extractDomain(pageUrl);
    if (domain.isEmpty()) return {};

    if (const QString *cached = m_memoryCache.object(domain))
        return *cached;

    QString localPath = cacheFileName(domain);
    if (QFile::exists(localPath))
        return localPath;

    return {};
}

// ── 核心方法 ──────────────────────────────────────────────────

// 异步获取 favicon
// @param pageUrl  网页 URL
// @param callback 回调函数，参数为本地缓存路径（空表示获取失败）
// 
// 流程：
// 1. 提取域名
// 2. 检查内存缓存
// 3. 检查本地文件缓存
// 4. 如果都没有，发起网络请求
// 5. 保存到本地缓存
// 6. 通过回调返回结果
void FaviconService::fetchFavicon(const QString &pageUrl,
                                   std::function<void(const QString &)> callback)
{
    QString domain = extractDomain(pageUrl);
    if (domain.isEmpty())
    {
        if (callback) callback({});
        return;
    }

    QString cached = getCachedFavicon(pageUrl);
    if (!cached.isEmpty())
    {
        if (callback) callback(cached);
        emit faviconReady(pageUrl, cached);
        return;
    }

    // 保存 callback 到队列，等抓取完成后调用
    m_pendingCallbacks[domain].append(callback);

    // 如果已经有正在进行的请求，不重复发起
    if (m_pendingRequests.contains(domain))
        return;
    m_pendingRequests.insert(domain);

    QStringList urls = buildFaviconUrls(domain);
    auto *attemptIndex = new int(0);

    // 逐次尝试 URL 列表
    tryNextUrl(domain, pageUrl, urls, attemptIndex);
}

// 递归尝试下一个 URL
// 如果当前 URL 失败，自动尝试下一个
// 所有 URL 都失败后，通知所有等待的回调
void FaviconService::tryNextUrl(const QString &domain, const QString &pageUrl,
                                 const QStringList &urls, int *attemptIndex)
{
    if (*attemptIndex >= urls.size())
    {
        // 全部尝试失败
        delete attemptIndex;
        m_pendingRequests.remove(domain);

        auto callbacks = m_pendingCallbacks.take(domain);
        for (auto &cb : callbacks) {
            if (cb) cb({});
        }
        emit faviconReady(pageUrl, {});
        return;
    }

    QNetworkRequest request(QUrl(urls[*attemptIndex]));
    request.setTransferTimeout(5000);

    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, domain, pageUrl, urls, attemptIndex]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray data = reply->readAll();
            QBuffer buf(&data);
            QImageReader reader(&buf);
            if (reader.canRead())
            {
                QString localPath = cacheFileName(domain);
                QFile file(localPath);
                if (file.open(QIODevice::WriteOnly))
                {
                    file.write(data);
                    file.close();
                    m_memoryCache.insert(domain, new QString(localPath));

                    delete attemptIndex;
                    m_pendingRequests.remove(domain);

                    auto callbacks = m_pendingCallbacks.take(domain);
                    for (auto &cb : callbacks) {
                        if (cb) cb(localPath);
                    }
                    emit faviconReady(pageUrl, localPath);
                    return;
                }
            }
        }

        (*attemptIndex)++;
        tryNextUrl(domain, pageUrl, urls, attemptIndex);
    });
}

// 清除所有缓存
// 包括内存缓存、待处理请求和本地文件
void FaviconService::clearCache()
{
    m_memoryCache.clear();
    m_pendingRequests.clear();
    m_pendingCallbacks.clear();
    QDir dir(cacheDirectory());
    dir.removeRecursively();
    QDir().mkpath(cacheDirectory());
}
