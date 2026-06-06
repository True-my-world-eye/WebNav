// IFolderRepository.h — 文件夹仓库接口
// 定义文件夹的层级管理操作

#pragma once
#include <QVector>
#include <optional>
#include "Folder.h"

class IFolderRepository
{
public:
    virtual ~IFolderRepository() = default;

    virtual QVector<Folder> getAll() = 0;
    virtual std::optional<Folder> getById(int id) = 0;
    virtual QVector<Folder> getByParent(int parentId) = 0;      // 获取子文件夹列表
    virtual QVector<Folder> getRootFolders() = 0;               // 获取根级文件夹

    virtual int insert(const Folder &folder) = 0;
    virtual bool update(const Folder &folder) = 0;
    virtual bool remove(int id) = 0;

    // 移动文件夹到新父节点（拖拽排序用）
    virtual bool moveFolder(int folderId, int newParentId, int newOrder) = 0;

    virtual int count() = 0;
};
