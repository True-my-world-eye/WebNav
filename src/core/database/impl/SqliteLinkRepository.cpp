// SqliteLinkRepository.cpp — SQLite 链接仓库实现
// 实现 ILinkRepository 接口，提供链接数据的 CRUD 操作
// 注意：所有 QString 字段在绑定前需检查 isNull()，避免绑定为 SQL NULL

#include "SqliteLinkRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteLinkRepository::SqliteLinkRepository(QSqlDatabase &db)
    : m_db(db)
{
}

// ── 辅助方法 ──────────────────────────────────────────────────

// 从当前查询行解析为 Link 对象
// 将数据库行数据转换为 Link 结构体，处理 NULL 值和日期格式
Link SqliteLinkRepository::rowToLink(const QSqlQuery &query) const
{
    Link link;
    link.id              = query.value("id").toInt();
    link.folderId        = query.value("folder_id").toInt();
    if (query.value("folder_id").isNull()) link.folderId = -1;

    link.title           = query.value("title").toString();
    link.url             = query.value("url").toString();
    link.description     = query.value("description").toString();
    link.notes           = query.value("notes").toString();
    link.faviconPath     = query.value("favicon_path").toString();
    link.thumbnailPath   = query.value("thumbnail_path").toString();
    link.visitCount      = query.value("visit_count").toInt();
    link.lastVisitedAt   = QDateTime::fromString(query.value("last_visited_at").toString(), Qt::ISODate);
    link.isBroken        = query.value("is_broken").toBool();
    link.createdAt       = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    link.updatedAt       = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    link.sortOrder       = query.value("sort_order").toInt();
    link.syncVersion     = query.value("sync_version").toInt();
    link.syncUpdatedAt   = QDateTime::fromString(query.value("sync_updated_at").toString(), Qt::ISODate);
    return link;
}

// ── 查询实现 ──────────────────────────────────────────────

// 获取所有链接
// 按 sort_order 升序、created_at 降序排列，确保手动排序优先
QVector<Link> SqliteLinkRepository::getAll()
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM links ORDER BY sort_order ASC, created_at DESC");
    while (query.next())
        results.append(rowToLink(query));
    return results;
}

// 根据 ID 获取单条链接
// 使用参数化查询防止 SQL 注入
std::optional<Link> SqliteLinkRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToLink(query);
    return std::nullopt;
}

// 获取指定文件夹下的所有链接
// folderId = -1 表示未分类链接
QVector<Link> SqliteLinkRepository::getByFolder(int folderId)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE folder_id = ? ORDER BY sort_order ASC, created_at DESC");
    query.addBindValue(folderId);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 搜索链接
// 使用 LIKE 模糊匹配标题、URL、描述三个字段
// %keyword% 模式支持任意位置匹配
QVector<Link> SqliteLinkRepository::search(const QString &keyword)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE title LIKE ? OR url LIKE ? OR description LIKE ? ORDER BY sort_order ASC, created_at DESC");
    QString pattern = "%" + keyword + "%";
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取最近添加的链接
// 按创建时间降序排列，返回指定数量
QVector<Link> SqliteLinkRepository::getRecent(int limit)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links ORDER BY sort_order ASC, created_at DESC LIMIT ?");
    query.addBindValue(limit);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取包含指定标签的所有链接
// 通过 link_tags 关联表进行多表连接查询
QVector<Link> SqliteLinkRepository::getByTag(int tagId)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT l.* FROM links l
        INNER JOIN link_tags lt ON l.id = lt.link_id
        WHERE lt.tag_id = ?
        ORDER BY l.sort_order ASC, l.created_at DESC
    )");
    query.addBindValue(tagId);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取同时满足文件夹和标签条件的链接
// 组合筛选条件，用于侧边栏的文件夹+标签联合筛选
QVector<Link> SqliteLinkRepository::getByFolderAndTag(int folderId, int tagId)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT l.* FROM links l
        INNER JOIN link_tags lt ON l.id = lt.link_id
        WHERE l.folder_id = ? AND lt.tag_id = ?
        ORDER BY l.sort_order ASC, l.created_at DESC
    )");
    query.addBindValue(folderId);
    query.addBindValue(tagId);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取访问次数最多的链接
// 按 visit_count 降序排列，用于"频繁访问"筛选
QVector<Link> SqliteLinkRepository::getMostVisited(int limit)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links ORDER BY visit_count DESC LIMIT ?");
    query.addBindValue(limit);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取所有失效链接
// is_broken = 1 表示链接已失效，用于侧边栏的失效链接筛选
QVector<Link> SqliteLinkRepository::getBroken()
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM links WHERE is_broken = 1 ORDER BY updated_at DESC");
    while (query.next())
        results.append(rowToLink(query));
    return results;
}

// ── 写入实现 ──────────────────────────────────────────────

// 插入新链接
// 注意：QString() 默认构造为 null string，需要显式转为空字符串
// 否则 Qt SQLite 驱动会将其绑定为 SQL NULL，导致 NOT NULL 约束失败
int SqliteLinkRepository::insert(const Link &link)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO links (folder_id, title, url, description, notes, favicon_path, thumbnail_path,
                          visit_count, last_visited_at, is_broken, sort_order, created_at, updated_at, sync_version)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, datetime("now"), datetime("now"), ?)
    )");
    query.addBindValue(link.folderId > 0 ? QVariant(link.folderId) : QVariant());
    query.addBindValue(link.title.isNull() ? QString("") : link.title);
    query.addBindValue(link.url);
    query.addBindValue(link.description.isNull() ? QString("") : link.description);
    query.addBindValue(link.notes.isNull() ? QString("") : link.notes);
    query.addBindValue(link.faviconPath.isNull() ? QString("") : link.faviconPath);
    query.addBindValue(link.thumbnailPath.isNull() ? QString("") : link.thumbnailPath);
    query.addBindValue(link.visitCount);
    query.addBindValue(link.lastVisitedAt.isValid() ? QVariant(link.lastVisitedAt.toString(Qt::ISODate)) : QVariant());
    query.addBindValue(link.isBroken ? 1 : 0);
    query.addBindValue(link.sortOrder);
    query.addBindValue(link.syncVersion);

    if (query.exec())
        return query.lastInsertId().toInt();

    qWarning() << "[SqliteLinkRepo] 插入失败:" << query.lastError().text();
    return -1;
}

// 更新链接信息
// 自动更新 updated_at 时间戳
// sync_version 用于同步时的冲突检测
bool SqliteLinkRepository::update(const Link &link)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE links SET folder_id=?, title=?, url=?, description=?, notes=?,
                         favicon_path=?, thumbnail_path=?, visit_count=?,
                         last_visited_at=?, is_broken=?, sort_order=?, updated_at=datetime("now"),
                         sync_version=?
        WHERE id=?
    )");
    query.addBindValue(link.folderId > 0 ? QVariant(link.folderId) : QVariant());
    query.addBindValue(link.title.isNull() ? QString("") : link.title);
    query.addBindValue(link.url);
    query.addBindValue(link.description.isNull() ? QString("") : link.description);
    query.addBindValue(link.notes.isNull() ? QString("") : link.notes);
    query.addBindValue(link.faviconPath.isNull() ? QString("") : link.faviconPath);
    query.addBindValue(link.thumbnailPath.isNull() ? QString("") : link.thumbnailPath);
    query.addBindValue(link.visitCount);
    query.addBindValue(link.lastVisitedAt.isValid() ? QVariant(link.lastVisitedAt.toString(Qt::ISODate)) : QVariant());
    query.addBindValue(link.isBroken ? 1 : 0);
    query.addBindValue(link.sortOrder);
    query.addBindValue(link.syncVersion);
    query.addBindValue(link.id);

    if (query.exec())
        return query.numRowsAffected() > 0;

    qWarning() << "[SqliteLinkRepo] 更新失败:" << query.lastError().text();
    return false;
}

// 删除指定 ID 的链接
// 注意：由于外键约束，关联的 link_tags 和 link_fields 记录会被自动删除
bool SqliteLinkRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM links WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// 批量删除多个链接
// 使用事务确保原子性：要么全部删除，要么全部回滚
bool SqliteLinkRepository::removeMultiple(const QVector<int> &ids)
{
    if (ids.isEmpty()) return true;

    m_db.transaction();
    QSqlQuery query(m_db);
    for (int id : ids)
    {
        query.prepare("DELETE FROM links WHERE id = ?");
        query.addBindValue(id);
        if (!query.exec())
        {
            m_db.rollback();
            return false;
        }
    }
    return m_db.commit();
}

// 获取指定时间后有变更的链接
// 用于增量同步，只获取本地变更的数据
QVector<Link> SqliteLinkRepository::getChangedSince(const QDateTime &since)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE updated_at > ? ORDER BY updated_at");
    query.addBindValue(since.toString(Qt::ISODate));
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

// 获取链接总数
// 用于状态栏显示
int SqliteLinkRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM links") && query.next())
        return query.value(0).toInt();
    return 0;
}

// 获取失效链接总数
// 用于侧边栏失效链接按钮显示
int SqliteLinkRepository::brokenCount()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM links WHERE is_broken = 1") && query.next())
        return query.value(0).toInt();
    return 0;
}

// 批量更新链接排序序号
// 用于拖拽排序和工具栏排序按钮
// 使用事务确保原子性
bool SqliteLinkRepository::reorderLinks(const QVector<QPair<int,int>> &orders)
{
    m_db.transaction();
    QSqlQuery query(m_db);
    query.prepare("UPDATE links SET sort_order=? WHERE id=?");
    for (const auto &pair : orders)
    {
        query.addBindValue(pair.second);
        query.addBindValue(pair.first);
        if (!query.exec()) { m_db.rollback(); return false; }
    }
    return m_db.commit();
}

// ── 附加字段存储 ──────────────────────────────────────────

// 保存链接的附加字段
// 采用"先删后插"策略：先删除该链接的所有旧字段，再插入新字段
// 使用事务确保原子性
bool SqliteLinkRepository::saveLinkFields(int linkId, const QVector<LinkField> &fields)
{
    // 先删除旧字段
    QSqlQuery del(m_db);
    del.prepare("DELETE FROM link_fields WHERE link_id = ?");
    del.addBindValue(linkId);
    del.exec();

    // 插入新字段
    m_db.transaction();
    QSqlQuery ins(m_db);
    for (const auto &f : fields)
    {
        ins.prepare("INSERT INTO link_fields (link_id, field_key, field_value, field_type, is_password, sort_order) "
                     "VALUES (?,?,?,?,?,?)");
        ins.addBindValue(linkId);
        ins.addBindValue(f.fieldKey);
        ins.addBindValue(f.fieldValue);
        ins.addBindValue(f.fieldType);
        ins.addBindValue(f.isPassword ? 1 : 0);
        ins.addBindValue(f.sortOrder);
        if (!ins.exec()) { m_db.rollback(); return false; }
    }
    return m_db.commit();
}

// 获取链接的所有附加字段
// 按 sort_order 排序，确保界面显示顺序一致
QVector<LinkField> SqliteLinkRepository::getLinkFields(int linkId)
{
    QVector<LinkField> results;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM link_fields WHERE link_id = ? ORDER BY sort_order");
    q.addBindValue(linkId);
    if (q.exec()) {
        while (q.next()) {
            LinkField f;
            f.id = q.value("id").toInt();
            f.linkId = q.value("link_id").toInt();
            f.fieldKey = q.value("field_key").toString();
            f.fieldValue = q.value("field_value").toString();
            f.fieldType = q.value("field_type").toInt();
            f.isPassword = q.value("is_password").toBool();
            f.sortOrder = q.value("sort_order").toInt();
            results.append(f);
        }
    }
    return results;
}
