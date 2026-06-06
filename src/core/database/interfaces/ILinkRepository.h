// ILinkRepository.h — 链接仓库接口
// 定义链接数据的持久化操作契约，本地 SQLite 和远程 API 共享此接口
// 后续可添加 RestApiLinkRepository 实现实现云端同步

#pragma once
#include <QVector>
#include <optional>
#include "Link.h"

// 链接仓库接口，所有持久化操作通过此接口完成
class ILinkRepository
{
public:
    virtual ~ILinkRepository() = default;

    // ── 查询 ──────────────────────────────────────────────

    // 获取所有链接，按创建时间倒序排列
    virtual QVector<Link> getAll() = 0;

    // 根据 ID 获取单条链接
    virtual std::optional<Link> getById(int id) = 0;

    // 获取指定文件夹下的所有链接
    virtual QVector<Link> getByFolder(int folderId) = 0;

    // 搜索链接，匹配标题/URL/备注（模糊搜索）
    virtual QVector<Link> search(const QString &keyword) = 0;

    // 获取最近添加的 N 条链接
    virtual QVector<Link> getRecent(int limit = 50) = 0;

    // 获取访问次数最多的 N 条链接
    virtual QVector<Link> getMostVisited(int limit = 50) = 0;

    // 获取所有失效链接
    virtual QVector<Link> getBroken() = 0;

    // ── 写入 ──────────────────────────────────────────────

    // 插入新链接，返回自增 ID
    virtual int insert(const Link &link) = 0;

    // 更新链接信息，返回是否成功
    virtual bool update(const Link &link) = 0;

    // 根据 ID 删除链接
    virtual bool remove(int id) = 0;

    // 批量删除链接
    virtual bool removeMultiple(const QVector<int> &ids) = 0;

    // ── 统计与同步 ────────────────────────────────────────

    // 获取自指定时间以来有变动的链接（供同步使用）
    virtual QVector<Link> getChangedSince(const QDateTime &since) = 0;

    // 获取链接总数
    virtual int count() = 0;

    // 获取失效链接数
    virtual int brokenCount() = 0;
};
