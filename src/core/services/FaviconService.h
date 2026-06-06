// FaviconService.h — Favicon 抓取服务
// 异步获取网页 favicon 并缓存到本地

#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QCache>
#include <functional>

class FaviconService : public QObject
{
    Q_OBJECT

public:
    explicit FaviconService(QObject *parent = nullptr);

    // 异步获取 favicon，通过回调返回本地缓存路径
    // @param url      网页 URL
    // @param callback 回调函数，参数为本地缓存路径（空表示获取失败）
    void fetchFavicon(const QString &pageUrl,
                      std::function<void(const QString &localPath)> callback);

    // 从缓存中同步获取已缓存的 favicon 路径，未缓存则返回空
    QString getCachedFavicon(const QString &pageUrl) const;

    // 清除所有缓存
    void clearCache();

    // 获取 favicon 缓存目录
    static QString cacheDirectory();

signals:
    // favicon 更新完成信号
    void faviconReady(const QString &pageUrl, const QString &localPath);

private:
    // 从 URL 提取域名
    static QString extractDomain(const QString &pageUrl);

    // 构建 favicon URL 列表（尝试多个位置）
    static QStringList buildFaviconUrls(const QString &domain);

    // 生成缓存文件名
    static QString cacheFileName(const QString &domain);

    QNetworkAccessManager *m_network;       // 网络请求管理器
    QCache<QString, QString> m_memoryCache; // 内存缓存（key=域名, value=本地路径）
};
