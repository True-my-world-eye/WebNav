// LinkCheckService.h — 死链检测服务
// 后台静默检测链接是否可访问

#pragma once
#include <QObject>
#include <QNetworkAccessManager>

class LinkCheckService : public QObject
{
    Q_OBJECT

public:
    explicit LinkCheckService(QObject *parent = nullptr);

    // 开始检测指定链接是否可达
    void checkLink(const QString &url);

    // 批量检测
    void checkLinks(const QStringList &urls);

signals:
    // 检测完成信号
    void linkCheckCompleted(const QString &url, bool isReachable, int statusCode);

private:
    QNetworkAccessManager *m_network;
};
