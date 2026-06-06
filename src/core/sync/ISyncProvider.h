// ISyncProvider.h — 同步提供者接口
// Phase 3 实现：支持 WebDAV / REST API / 本地文件同步

#pragma once
#include <QString>
#include <QJsonObject>

class ISyncProvider
{
public:
    virtual ~ISyncProvider() = default;

    // 推送本地数据到远程
    virtual bool push(const QJsonObject &data) = 0;

    // 从远程拉取数据
    virtual QJsonObject pull() = 0;

    // 获取上次同步时间
    virtual QString lastSyncTime() const = 0;

    // 判断同步是否可用
    virtual bool isAvailable() const = 0;
};
