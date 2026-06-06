// LinkCheckService.cpp

#include "LinkCheckService.h"
#include <QNetworkReply>
#include <QNetworkRequest>

LinkCheckService::LinkCheckService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
}

void LinkCheckService::checkLink(const QString &url)
{
    QNetworkRequest request(QUrl(url));
    request.setTransferTimeout(10000);          // 10 秒超时
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    // 使用 HEAD 请求（只获取响应头，不下载内容）
    QNetworkReply *reply = m_network->head(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        reply->deleteLater();

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        bool isReachable = (reply->error() == QNetworkReply::NoError);

        emit linkCheckCompleted(url, isReachable, statusCode);
    });
}

void LinkCheckService::checkLinks(const QStringList &urls)
{
    for (const QString &url : urls)
    {
        checkLink(url);
    }
}
