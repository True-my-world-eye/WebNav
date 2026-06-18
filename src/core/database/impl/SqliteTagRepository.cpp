// SqliteTagRepository.cpp — SQLite 标签仓库实现
// 实现 ITagRepository 接口，提供标签的 CRUD 操作和链接-标签关联管理
// 标签名称全局唯一，支持彩色标签显示

#include "SqliteTagRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteTagRepository::SqliteTagRepository(QSqlDatabase &db)
    : m_db(db)
{
}

// ── 辅助方法 ──────────────────────────────────────────────────

// 从当前查询行解析为 Tag 对象
Tag SqliteTagRepository::rowToTag(const QSqlQuery &query) const
{
    Tag t;
    t.id        = query.value("id").toInt();
    t.name      = query.value("name").toString();
    t.color     = query.value("color").toString();
    t.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    return t;
}

// ── 查询实现 ──────────────────────────────────────────────

// 获取所有标签
// 按名称升序排列
QVector<Tag> SqliteTagRepository::getAll()
{
    QVector<Tag> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM tags ORDER BY name ASC");
    while (query.next()) results.append(rowToTag(query));
    return results;
}

// 根据 ID 获取单个标签
std::optional<Tag> SqliteTagRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tags WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToTag(query);
    return std::nullopt;
}

// 根据名称获取标签
// 用于批量打标签时查找已存在的标签
std::optional<Tag> SqliteTagRepository::getByName(const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tags WHERE name = ?");
    query.addBindValue(name);
    if (query.exec() && query.next())
        return rowToTag(query);
    return std::nullopt;
}

// ── 写入实现 ──────────────────────────────────────────────

// 插入新标签
// 注意：标签名称有 UNIQUE 约束，重复名称会插入失败
int SqliteTagRepository::insert(const Tag &tag)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO tags (name, color, created_at) VALUES (?, ?, datetime(\"now\"))");
    query.addBindValue(tag.name);
    query.addBindValue(tag.color);
    if (query.exec())
        return query.lastInsertId().toInt();
    qWarning() << "[SqliteTagRepo] 插入失败:" << query.lastError().text();
    return -1;
}

// 更新标签名称和颜色
bool SqliteTagRepository::update(const Tag &tag)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE tags SET name=?, color=? WHERE id=?");
    query.addBindValue(tag.name);
    query.addBindValue(tag.color);
    query.addBindValue(tag.id);
    return query.exec();
}

// 删除标签
// 注意：由于外键约束，关联的 link_tags 记录会被自动删除
bool SqliteTagRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tags WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// ── 链接-标签关联操作 ──────────────────────────────────────

// 为链接添加标签
// 使用 INSERT OR IGNORE 防止重复添加
bool SqliteTagRepository::addTagToLink(int linkId, int tagId)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO link_tags (link_id, tag_id) VALUES (?, ?)");
    query.addBindValue(linkId);
    query.addBindValue(tagId);
    return query.exec();
}

// 移除链接的某个标签
bool SqliteTagRepository::removeTagFromLink(int linkId, int tagId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM link_tags WHERE link_id = ? AND tag_id = ?");
    query.addBindValue(linkId);
    query.addBindValue(tagId);
    return query.exec();
}

// 获取链接的所有标签
// 通过 link_tags 关联表进行多表连接查询
QVector<Tag> SqliteTagRepository::getTagsForLink(int linkId)
{
    QVector<Tag> results;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT t.* FROM tags t
        INNER JOIN link_tags lt ON t.id = lt.tag_id
        WHERE lt.link_id = ?
        ORDER BY t.name ASC
    )");
    query.addBindValue(linkId);
    if (query.exec())
    {
        while (query.next()) results.append(rowToTag(query));
    }
    return results;
}

// 获取包含某个标签的所有链接 ID
// 用于标签筛选功能
QVector<int> SqliteTagRepository::getLinkIdsForTag(int tagId)
{
    QVector<int> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT link_id FROM link_tags WHERE tag_id = ?");
    query.addBindValue(tagId);
    if (query.exec())
    {
        while (query.next())
            results.append(query.value(0).toInt());
    }
    return results;
}

// 获取标签总数
int SqliteTagRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM tags") && query.next())
        return query.value(0).toInt();
    return 0;
}
