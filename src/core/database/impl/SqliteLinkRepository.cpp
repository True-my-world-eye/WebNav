// SqliteLinkRepository.cpp

#include "SqliteLinkRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteLinkRepository::SqliteLinkRepository(QSqlDatabase &db)
    : m_db(db)
{
}

// 从当前查询行解析为 Link 对象
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
    link.syncVersion     = query.value("sync_version").toInt();
    link.syncUpdatedAt   = QDateTime::fromString(query.value("sync_updated_at").toString(), Qt::ISODate);
    return link;
}

// ── 查询实现 ──────────────────────────────────────────────

QVector<Link> SqliteLinkRepository::getAll()
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM links ORDER BY created_at DESC");
    while (query.next())
        results.append(rowToLink(query));
    return results;
}

std::optional<Link> SqliteLinkRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToLink(query);
    return std::nullopt;
}

QVector<Link> SqliteLinkRepository::getByFolder(int folderId)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE folder_id = ? ORDER BY created_at DESC");
    query.addBindValue(folderId);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

QVector<Link> SqliteLinkRepository::search(const QString &keyword)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links WHERE title LIKE ? OR url LIKE ? OR description LIKE ? ORDER BY created_at DESC");
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

QVector<Link> SqliteLinkRepository::getRecent(int limit)
{
    QVector<Link> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM links ORDER BY created_at DESC LIMIT ?");
    query.addBindValue(limit);
    if (query.exec())
    {
        while (query.next())
            results.append(rowToLink(query));
    }
    return results;
}

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

int SqliteLinkRepository::insert(const Link &link)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO links (folder_id, title, url, description, notes, favicon_path, thumbnail_path,
                          visit_count, last_visited_at, is_broken, created_at, updated_at, sync_version)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, datetime("now"), datetime("now"), ?)
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
    query.addBindValue(link.syncVersion);

    if (query.exec())
        return query.lastInsertId().toInt();

    qWarning() << "[SqliteLinkRepo] 插入失败:" << query.lastError().text();
    return -1;
}

bool SqliteLinkRepository::update(const Link &link)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE links SET folder_id=?, title=?, url=?, description=?, notes=?,
                         favicon_path=?, thumbnail_path=?, visit_count=?,
                         last_visited_at=?, is_broken=?, updated_at=datetime("now"),
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
    query.addBindValue(link.syncVersion);
    query.addBindValue(link.id);

    if (query.exec())
        return query.numRowsAffected() > 0;

    qWarning() << "[SqliteLinkRepo] 更新失败:" << query.lastError().text();
    return false;
}

bool SqliteLinkRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM links WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

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

int SqliteLinkRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM links") && query.next())
        return query.value(0).toInt();
    return 0;
}

int SqliteLinkRepository::brokenCount()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM links WHERE is_broken = 1") && query.next())
        return query.value(0).toInt();
    return 0;
}

// ── 附加字段存储 ──────────────────────────────────────────

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
