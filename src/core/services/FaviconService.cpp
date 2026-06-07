// FaviconService.cpp — Favicon 抓取服务实现

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
    , m_memoryCache(100)
{
}

QString FaviconService::cacheDirectory()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                  + "/favicons/";
    QDir().mkpath(dir);
    return dir;
}

QString FaviconService::extractDomain(const QString &pageUrl)
{
    QUrl url(pageUrl);
    return url.host().toLower();
}

QStringList FaviconService::buildFaviconUrls(const QString &domain)
{
    return {
        QString("https://%1/favicon.ico").arg(domain),
        QString("https://%1/apple-touch-icon.png").arg(domain),
        QString("https://%1/apple-touch-icon-precomposed.png").arg(domain),
        QString("https://www.google.com/s2/favicons?domain=%1&sz=64").arg(domain),
    };
}

QString FaviconService::cacheFileName(const QString &domain)
{
    QByteArray hash = QCryptographicHash::hash(domain.toUtf8(), QCryptographicHash::Md5);
    return cacheDirectory() + hash.toHex() + ".png";
}

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

void FaviconService::clearCache()
{
    m_memoryCache.clear();
    m_pendingRequests.clear();
    m_pendingCallbacks.clear();
    QDir dir(cacheDirectory());
    dir.removeRecursively();
    QDir().mkpath(cacheDirectory());
}
