// SyncManager.h — 同步管理器
// 负责同步调度、冲突合并、数据序列化

#pragma once
#include <QObject>
#include <memory>
#include "ISyncProvider.h"
#include "interfaces/ILinkRepository.h"
#include "interfaces/IFolderRepository.h"
#include "interfaces/ITagRepository.h"

class SyncManager : public QObject
{
    Q_OBJECT

public:
    explicit SyncManager(QObject *parent = nullptr);

    // 设置同步提供者
    void setSyncProvider(std::unique_ptr<ISyncProvider> provider);

    // 设置数据仓库引用
    void setRepositories(ILinkRepository *links, IFolderRepository *folders, ITagRepository *tags);

    // 执行一次完整同步
    void syncNow();

    // 获取上次同步状态
    QString lastSyncStatus() const;

signals:
    void syncStarted();
    void syncCompleted(bool success, const QString &message);

private:
    std::unique_ptr<ISyncProvider> m_provider;
    ILinkRepository   *m_linkRepo = nullptr;
    IFolderRepository *m_folderRepo = nullptr;
    ITagRepository    *m_tagRepo = nullptr;
    QString m_lastStatus;
};
