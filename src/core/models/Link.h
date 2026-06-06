// Link.h — 链接数据模型
// 表示一条被收藏的网页链接，包含标题、URL、所属文件夹、访问统计等核心字段

#pragma once
#include <QString>
#include <QDateTime>
#include <optional>

// 链接数据结构，纯数据容器，不含业务逻辑
struct Link
{
    int id = -1;                        // 数据库主键，-1 表示尚未持久化
    int folderId = -1;                  // 所属文件夹 ID，-1 表示未分类
    QString title;                      // 网页标题（自动抓取或手动填写）
    QString url;                        // 链接地址（必填字段）
    QString description;                // 简短描述
    QString notes;                      // 长备注/摘录
    QString faviconPath;                // favicon 本地缓存路径
    QString thumbnailPath;              // 截图缩略图本地缓存路径
    int visitCount = 0;                 // 访问次数
    QDateTime lastVisitedAt;            // 最近访问时间
    bool isBroken = false;              // 是否已失效（死链检测结果）
    QDateTime createdAt;                // 创建时间
    QDateTime updatedAt;                // 最后修改时间
    int syncVersion = 1;                // 同步版本号（冲突检测用）
    QDateTime syncUpdatedAt;            // 同步专用时间戳

    // 判断是否为有效链接（ID 非 -1）
    bool isValid() const { return id > 0 && !url.isEmpty(); }
};
