// SqliteTagRepository.cpp

#include "SqliteTagRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteTagRepository::SqliteTagRepository(QSqlDatabase &db)
    : m_db(db)
{
}

Tag SqliteTagRepository::rowToTag(const QSqlQuery &query) const
{
    Tag t;
    t.id        = query.value("id").toInt();
    t.name      = query.value("name").toString();
    t.color     = query.value("color").toString();
    t.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    return t;
}

QVector<Tag> SqliteTagRepository::getAll()
{
    QVector<Tag> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM tags ORDER BY name ASC");
    while (query.next()) results.append(rowToTag(query));
    return results;
}

std::optional<Tag> SqliteTagRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tags WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToTag(query);
    return std::nullopt;
}

std::optional<Tag> SqliteTagRepository::getByName(const QString &name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM tags WHERE name = ?");
    query.addBindValue(name);
    if (query.exec() && query.next())
        return rowToTag(query);
    return std::nullopt;
}

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

bool SqliteTagRepository::update(const Tag &tag)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE tags SET name=?, color=? WHERE id=?");
    query.addBindValue(tag.name);
    query.addBindValue(tag.color);
    query.addBindValue(tag.id);
    return query.exec();
}

bool SqliteTagRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tags WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool SqliteTagRepository::addTagToLink(int linkId, int tagId)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO link_tags (link_id, tag_id) VALUES (?, ?)");
    query.addBindValue(linkId);
    query.addBindValue(tagId);
    return query.exec();
}

bool SqliteTagRepository::removeTagFromLink(int linkId, int tagId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM link_tags WHERE link_id = ? AND tag_id = ?");
    query.addBindValue(linkId);
    query.addBindValue(tagId);
    return query.exec();
}

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

int SqliteTagRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM tags") && query.next())
        return query.value(0).toInt();
    return 0;
}
