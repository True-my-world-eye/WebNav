// Folder.h — 文件夹数据模型
// 用于对链接进行树形分类管理，支持无限层级嵌套

#pragma once
#include <QString>
#include <QDateTime>

// 文件夹数据结构
struct Folder
{
    int id = -1;                        // 数据库主键
    QString name;                       // 文件夹名称
    int parentId = -1;                  // 父文件夹 ID，-1 表示根级文件夹
    int sortOrder = 0;                  // 同级文件夹的排序序号
    QDateTime createdAt;                // 创建时间
    QDateTime updatedAt;                // 最后修改时间

    bool isValid() const { return id > 0 && !name.isEmpty(); }
};
