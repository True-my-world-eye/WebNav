// BookmarkImporter.h — 书签导入服务
// 解析 Chrome/Firefox/Edge 导出的 Netscape HTML 书签文件

#pragma once
#include <QString>
#include <QVector>
#include "core/models/Link.h"
#include "core/models/Folder.h"

// 导入结果结构
struct ImportResult
{
    int importedCount = 0;      // 成功导入的链接数
    int skippedCount = 0;      // 因重复跳过的链接数
    int folderCount = 0;       // 创建的文件夹数
    QString errorMessage;      // 错误信息
};

class BookmarkImporter
{
public:
    BookmarkImporter() = default;

    // 从 HTML 文件导入书签
    // @param filePath  HTML 书签文件路径
    // @return 导入结果
    ImportResult importFromFile(const QString &filePath);

    // 从 HTML 字符串导入书签
    ImportResult importFromHtml(const QString &htmlContent);

    // 获取已解析的链接列表
    QVector<Link> parsedLinks() const { return m_links; }

    // 获取已解析的文件夹列表
    QVector<Folder> parsedFolders() const { return m_folders; }

private:
    // 解析 Netscape HTML 格式的书签结构
    void parseNetscapeHtml(const QString &html);

    QVector<Link>   m_links;        // 解析出的链接
    QVector<Folder> m_folders;      // 解析出的文件夹
};
