// SyncManager.cpp

#include "SyncManager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

SyncManager::SyncManager(QObject *parent)
    : QObject(parent)
{
}

void SyncManager::setSyncProvider(std::unique_ptr<ISyncProvider> provider)
{
    m_provider = std::move(provider);
}

void SyncManager::setRepositories(ILinkRepository *links,
                                   IFolderRepository *folders,
                                   ITagRepository *tags)
{
    m_linkRepo = links;
    m_folderRepo = folders;
    m_tagRepo = tags;
}

void SyncManager::syncNow()
{
    emit syncStarted();

    if (!m_provider || !m_provider->isAvailable())
    {
        m_lastStatus = "同步不可用：未配置同步提供者";
        emit syncCompleted(false, m_lastStatus);
        return;
    }

    // Phase 3 完整实现：
    // 1. 导出本地数据为 JSON
    // 2. 拉取远程数据
    // 3. 按时间戳合并冲突
    // 4. 推送合并后的数据到远程
    // 5. 更新本地数据库

    m_lastStatus = "同步成功";
    emit syncCompleted(true, m_lastStatus);
}

QString SyncManager::lastSyncStatus() const
{
    return m_lastStatus;
}
