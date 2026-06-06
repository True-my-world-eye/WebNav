// ITagRepository.h — 标签仓库接口

#pragma once
#include <QVector>
#include <optional>
#include "Tag.h"

class ITagRepository
{
public:
    virtual ~ITagRepository() = default;

    virtual QVector<Tag> getAll() = 0;
    virtual std::optional<Tag> getById(int id) = 0;
    virtual std::optional<Tag> getByName(const QString &name) = 0;

    virtual int insert(const Tag &tag) = 0;
    virtual bool update(const Tag &tag) = 0;
    virtual bool remove(int id) = 0;

    // 为链接添加标签
    virtual bool addTagToLink(int linkId, int tagId) = 0;

    // 移除链接的某个标签
    virtual bool removeTagFromLink(int linkId, int tagId) = 0;

    // 获取链接的所有标签
    virtual QVector<Tag> getTagsForLink(int linkId) = 0;

    // 获取包含某个标签的所有链接 ID
    virtual QVector<int> getLinkIdsForTag(int tagId) = 0;

    virtual int count() = 0;
};
