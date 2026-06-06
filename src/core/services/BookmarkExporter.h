// BookmarkExporter.h — 书签导出服务
// 将应用内的链接导出为标准 Netscape HTML 书签文件

#pragma once
#include <QString>
#include <QVector>
#include "Link.h"
#include "Folder.h"

class BookmarkExporter
{
public:
    BookmarkExporter() = default;

    // 导出为 Netscape HTML 书签文件
    // @param filePath   导出目标路径
    // @param links      要导出的链接列表
    // @param folders    涉及的文件夹列表
    // @return 是否成功
    bool exportToFile(const QString &filePath,
                      const QVector<Link> &links,
                      const QVector<Folder> &folders);

    // 导出为 Markdown 文件（Phase 3）
    bool exportToMarkdown(const QString &filePath,
                          const QVector<Link> &links,
                          const QVector<Folder> &folders);
};
