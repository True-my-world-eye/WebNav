#pragma once
#include "../interfaces/ILinkRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class SqliteLinkRepository : public ILinkRepository
{
public:
    explicit SqliteLinkRepository(QSqlDatabase &db);
    QVector<Link> getAll() override;
    std::optional<Link> getById(int id) override;
    QVector<Link> getByFolder(int folderId) override;
    QVector<Link> search(const QString &keyword) override;
    QVector<Link> getRecent(int limit) override;
    QVector<Link> getMostVisited(int limit) override;
    QVector<Link> getBroken() override;
    int insert(const Link &link) override;
    bool update(const Link &link) override;
    bool remove(int id) override;
    bool removeMultiple(const QVector<int> &ids) override;
    QVector<Link> getChangedSince(const QDateTime &since) override;
    int count() override;
    int brokenCount() override;
private:
    Link rowToLink(const QSqlQuery &query) const;
    QSqlDatabase &m_db;
};
