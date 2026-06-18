// SqliteFolderRepository.cpp — SQLite 文件夹仓库实现
// 实现 IFolderRepository 接口，提供文件夹的层级管理操作
// 支持无限层级嵌套，通过 parent_id 字段建立父子关系

#include "SqliteFolderRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SqliteFolderRepository::SqliteFolderRepository(QSqlDatabase &db)
    : m_db(db)
{
}

// ── 辅助方法 ──────────────────────────────────────────────────

// 从当前查询行解析为 Folder 对象
// 处理 parent_id 为 NULL 的情况（根级文件夹）
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

// ── 查询实现 ──────────────────────────────────────────────

// 获取所有文件夹
// 按排序序号升序、名称升序排列
QVector<Folder> SqliteFolderRepository::getAll()
{
    QVector<Folder> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM folders ORDER BY sort_order ASC, name ASC");
    while (query.next()) results.append(rowToFolder(query));
    return results;
}

// 根据 ID 获取单个文件夹
std::optional<Folder> SqliteFolderRepository::getById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM folders WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return rowToFolder(query);
    return std::nullopt;
}

// 获取指定父文件夹的子文件夹
// 用于递归构建文件夹树
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

// 获取根级文件夹（parent_id IS NULL）
// 用于构建文件夹树的顶层节点
QVector<Folder> SqliteFolderRepository::getRootFolders()
{
    QVector<Folder> results;
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM folders WHERE parent_id IS NULL ORDER BY sort_order ASC, name ASC");
    while (query.next()) results.append(rowToFolder(query));
    return results;
}

// ── 写入实现 ──────────────────────────────────────────────

// 插入新文件夹
// parentId > 0 表示子文件夹，否则为根级文件夹
int SqliteFolderRepository::insert(const Folder &folder)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO folders (name, parent_id, sort_order, created_at, updated_at)
        VALUES (?, ?, ?, datetime("now"), datetime("now"))
    )");
    query.addBindValue(folder.name);
    query.addBindValue(folder.parentId > 0 ? QVariant(folder.parentId) : QVariant());
    query.addBindValue(folder.sortOrder);
    if (query.exec())
        return query.lastInsertId().toInt();
    qWarning() << "[SqliteFolderRepo] 插入失败:" << query.lastError().text();
    return -1;
}

// 更新文件夹名称
// 自动更新 updated_at 时间戳
bool SqliteFolderRepository::update(const Folder &folder)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE folders SET name=?, updated_at=datetime(\"now\") WHERE id=?");
    query.addBindValue(folder.name);
    query.addBindValue(folder.id);
    return query.exec() && query.numRowsAffected() > 0;
}

// 删除文件夹
// 注意：由于外键约束（ON DELETE CASCADE），子文件夹会被自动删除
bool SqliteFolderRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM folders WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// 移动文件夹到新的父节点
// 用于拖拽排序，更新 parent_id 和 sort_order
bool SqliteFolderRepository::moveFolder(int folderId, int newParentId, int newOrder)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE folders SET parent_id=?, sort_order=?, updated_at=datetime(\"now\") WHERE id=?");
    query.addBindValue(newParentId > 0 ? QVariant(newParentId) : QVariant());
    query.addBindValue(newOrder);
    query.addBindValue(folderId);
    return query.exec();
}

// 获取文件夹总数
int SqliteFolderRepository::count()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM folders") && query.next())
        return query.value(0).toInt();
    return 0;
}
