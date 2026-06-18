// ILinkRepository.h — 链接仓库接口
// 定义链接数据的访问操作，遵循 Repository 模式
// 通过接口抽象实现依赖倒置，UI 层不直接依赖具体数据库实现
// 为后续远程同步预留扩展能力

#pragma once
#include <QVector>
#include <QPair>
#include <QString>
#include <QDateTime>
#include <optional>
#include "Link.h"
#include "LinkField.h"

class ILinkRepository
{
public:
    virtual ~ILinkRepository() = default;

    // ── 查询操作 ──────────────────────────────────────────────
    
    // 获取所有链接，按排序序号和创建时间降序
    virtual QVector<Link> getAll() = 0;
    
    // 根据 ID 获取单条链接，不存在则返回 std::nullopt
    virtual std::optional<Link> getById(int id) = 0;
    
    // 获取指定文件夹下的所有链接
    virtual QVector<Link> getByFolder(int folderId) = 0;
    
    // 搜索链接（匹配标题、URL、描述）
    // @param keyword  搜索关键词，支持模糊匹配
    virtual QVector<Link> search(const QString &keyword) = 0;
    
    // 获取包含指定标签的所有链接
    virtual QVector<Link> getByTag(int tagId) = 0;
    
    // 获取同时满足文件夹和标签条件的链接
    virtual QVector<Link> getByFolderAndTag(int folderId, int tagId) = 0;
    
    // 获取最近添加的链接
    // @param limit  返回数量上限，默认50
    virtual QVector<Link> getRecent(int limit = 50) = 0;
    
    // 获取访问次数最多的链接
    // @param limit  返回数量上限，默认50
    virtual QVector<Link> getMostVisited(int limit = 50) = 0;
    
    // 获取所有失效链接（is_broken = 1）
    virtual QVector<Link> getBroken() = 0;

    // ── 写入操作 ──────────────────────────────────────────────
    
    // 插入新链接，返回新生成的 ID（失败返回 -1）
    virtual int insert(const Link &link) = 0;
    
    // 更新链接信息，返回是否成功
    virtual bool update(const Link &link) = 0;
    
    // 删除指定 ID 的链接
    virtual bool remove(int id) = 0;
    
    // 批量删除多个链接，使用事务确保原子性
    virtual bool removeMultiple(const QVector<int> &ids) = 0;
    
    // 批量更新链接排序序号
    // @param orders  QPair<链接ID, 新排序序号> 的列表
    virtual bool reorderLinks(const QVector<QPair<int,int>> &orders) = 0;

    // ── 同步相关 ──────────────────────────────────────────────
    
    // 获取指定时间后有变更的链接（用于增量同步）
    virtual QVector<Link> getChangedSince(const QDateTime &since) = 0;
    
    // 获取链接总数
    virtual int count() = 0;
    
    // 获取失效链接总数
    virtual int brokenCount() = 0;

    // ── 附加字段操作 ──────────────────────────────────────────
    
    // 保存链接的附加字段（先删除旧字段，再插入新字段）
    virtual bool saveLinkFields(int linkId, const QVector<LinkField> &fields) = 0;
    
    // 获取链接的所有附加字段，按排序序号排列
    virtual QVector<LinkField> getLinkFields(int linkId) = 0;
};
