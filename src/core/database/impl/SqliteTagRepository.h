// SqliteTagRepository.h — 标签仓库的 SQLite 实现

#pragma once
#include "../interfaces/ITagRepository.h"
#include <QSqlDatabase>

class SqliteTagRepository : public ITagRepository
{
public:
    explicit SqliteTagRepository(QSqlDatabase &db);

    QVector<Tag> getAll() override;
    std::optional<Tag> getById(int id) override;
    std::optional<Tag> getByName(const QString &name) override;

    int insert(const Tag &tag) override;
    bool update(const Tag &tag) override;
    bool remove(int id) override;

    bool addTagToLink(int linkId, int tagId) override;
    bool removeTagFromLink(int linkId, int tagId) override;
    QVector<Tag> getTagsForLink(int linkId) override;
    QVector<int> getLinkIdsForTag(int tagId) override;

    int count() override;

private:
    Tag rowToTag(const QSqlQuery &query) const;
    QSqlDatabase &m_db;
};
