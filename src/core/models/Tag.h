// Tag.h — 标签数据模型
// 用于对链接进行灵活的标签分类，一条链接可打多个标签

#pragma once
#include <QString>
#include <QDateTime>

// 标签数据结构
struct Tag
{
    int id = -1;                        // 数据库主键
    QString name;                       // 标签名称（全局唯一）
    QString color = "#5B9BD5";          // 标签颜色，十六进制格式
    QDateTime createdAt;                // 创建时间

    bool isValid() const { return id > 0 && !name.isEmpty(); }
};
