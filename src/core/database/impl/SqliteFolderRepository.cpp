// SqliteFolderRepository.cpp

#include "SqliteFolderRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteFolderRepository::SqliteFolderRepository(QSqlDatabase &db)
    : m_db(db)
{
}

Folder SqliteFolderRepository::rowToFolder(const QSqlQuery &query) const
{
    Folder f;
    f.id        = query.value("id").toInt();
    f.name      = query.value("name").toString();
    f.parentId  = query.value("parent_id").isNull() ? -1 : query.value("parent_id").toInt();
    f.sortOrder = query.value("sort_order").toInt();
    f.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    f.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return f;
}

QVector<Folder> SqliteFolderRepository::getAll()
{
    QVector<Folder> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM folders ORDER BY sort_order ASC, name ASC");
    while (query.next()) results.append(rowToFolder(query));
    return results;
}

std::optional<Folder> SqliteFolderRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM folders WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToFolder(query);
    return std::nullopt;
}

QVector<Folder> SqliteFolderRepository::getByParent(int parentId)
{
    QVector<Folder> results;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM folders WHERE parent_id = ? ORDER BY sort_order ASC, name ASC");
    query.addBindValue(parentId);
    if (query.exec())
    {
        while (query.next()) results.append(rowToFolder(query));
    }
    return results;
}

QVector<Folder> SqliteFolderRepository::getRootFolders()
{
    QVector<Folder> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM folders WHERE parent_id IS NULL ORDER BY sort_order ASC, name ASC");
    while (query.next()) results.append(rowToFolder(query));
    return results;
}

int SqliteFolderRepository::insert(const Folder &folder)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO folders (name, parent_id, sort_order, created_at, updated_at)
        VALUES (?, ?, ?, datetime("now"), datetime("now"))
    )");
    query.addBindValue(folder.name);
    query.addBindValue(folder.parentId > 0 ? folder.parentId : QVariant(QMetaType::Int));
    query.addBindValue(folder.sortOrder);
    if (query.exec())
        return query.lastInsertId().toInt();
    qWarning() << "[SqliteFolderRepo] 插入失败:" << query.lastError().text();
    return -1;
}

bool SqliteFolderRepository::update(const Folder &folder)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE folders SET name=?, updated_at=datetime(\"now\") WHERE id=?");
    query.addBindValue(folder.name);
    query.addBindValue(folder.id);
    return query.exec() && query.numRowsAffected() > 0;
}

bool SqliteFolderRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM folders WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool SqliteFolderRepository::moveFolder(int folderId, int newParentId, int newOrder)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE folders SET parent_id=?, sort_order=?, updated_at=datetime(\"now\") WHERE id=?");
    query.addBindValue(newParentId > 0 ? newParentId : QVariant(QMetaType::Int));
    query.addBindValue(newOrder);
    query.addBindValue(folderId);
    return query.exec();
}

int SqliteFolderRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM folders") && query.next())
        return query.value(0).toInt();
    return 0;
}
