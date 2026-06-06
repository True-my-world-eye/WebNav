#pragma once
#include <QVector>
#include <QString>
#include <QDateTime>
#include <optional>
#include "Link.h"
#include "LinkField.h"

class ILinkRepository
{
public:
    virtual ~ILinkRepository() = default;

    virtual QVector<Link> getAll() = 0;
    virtual std::optional<Link> getById(int id) = 0;
    virtual QVector<Link> getByFolder(int folderId) = 0;
    virtual QVector<Link> search(const QString &keyword) = 0;
    virtual QVector<Link> getRecent(int limit = 50) = 0;
    virtual QVector<Link> getMostVisited(int limit = 50) = 0;
    virtual QVector<Link> getBroken() = 0;

    virtual int insert(const Link &link) = 0;
    virtual bool update(const Link &link) = 0;
    virtual bool remove(int id) = 0;
    virtual bool removeMultiple(const QVector<int> &ids) = 0;

    virtual QVector<Link> getChangedSince(const QDateTime &since) = 0;
    virtual int count() = 0;
    virtual int brokenCount() = 0;

    virtual bool saveLinkFields(int linkId, const QVector<LinkField> &fields) = 0;
    virtual QVector<LinkField> getLinkFields(int linkId) = 0;
};
