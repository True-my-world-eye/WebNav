// SqliteFolderRepository.h — 文件夹仓库的 SQLite 实现

#pragma once
#include "../interfaces/IFolderRepository.h"
#include <QSqlDatabase>

class SqliteFolderRepository : public IFolderRepository
{
public:
    explicit SqliteFolderRepository(QSqlDatabase &db);

    QVector<Folder> getAll() override;
    std::optional<Folder> getById(int id) override;
    QVector<Folder> getByParent(int parentId) override;
    QVector<Folder> getRootFolders() override;

    int insert(const Folder &folder) override;
    bool update(const Folder &folder) override;
    bool remove(int id) override;
    bool moveFolder(int folderId, int newParentId, int newOrder) override;

    int count() override;

private:
    Folder rowToFolder(const QSqlQuery &query) const;
    QSqlDatabase &m_db;
};
