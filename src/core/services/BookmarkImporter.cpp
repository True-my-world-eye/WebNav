// BookmarkImporter.cpp

#include "BookmarkImporter.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QUrl>
#include <QDebug>

ImportResult BookmarkImporter::importFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {0, 0, 0, QString("无法打开文件: %1").arg(filePath)};
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    return importFromHtml(content);
}

ImportResult BookmarkImporter::importFromHtml(const QString &htmlContent)
{
    m_links.clear();
    m_folders.clear();
    parseNetscapeHtml(htmlContent);

    ImportResult result;
    result.importedCount = m_links.size();
    result.folderCount = m_folders.size();
    return result;
}

// 使用正则表达式逐行解析 Netscape 书签 HTML
// 格式示例：
//   <DT><H3>文件夹名</H3>
//   <DL><p>
//       <DT><A HREF="https://..." ADD_DATE="..." TAGS="...">标题</A>
//   </DL><p>
void BookmarkImporter::parseNetscapeHtml(const QString &html)
{
    // 提取 <META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=...">
    // 这里默认 UTF-8 处理

    QStringList lines = html.split("\n");

    // 用栈追踪当前文件夹层级
    struct FolderNode {
        QString name;
        int id;
        int parentId;
    };
    QVector<FolderNode> folderStack;
    int nextFolderId = 1;

    // 临时跟踪当前 H3（文件夹标题）后是否有 DL 块
    QString pendingFolderName;

    for (const QString &rawLine : lines)
    {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        // 匹配文件夹标题: <DT><H3 ...>名称</H3>
        static QRegularExpression folderRe(R"(<DT><H3[^>]*>(.*?)</H3>)", QRegularExpression::CaseInsensitiveOption);
        auto folderMatch = folderRe.match(line);
        if (folderMatch.hasMatch())
        {
            pendingFolderName = folderMatch.captured(1).trimmed();
            continue;
        }

        // 匹配 DL 开始（表示进入文件夹层级）
        if (line.contains("<DL", Qt::CaseInsensitive) && !pendingFolderName.isEmpty())
        {
            // 创建新文件夹节点
            int parentId = folderStack.isEmpty() ? -1 : folderStack.last().id;
            FolderNode node{pendingFolderName, nextFolderId++, parentId};

            Folder f;
            f.id = node.id;
            f.name = node.name;
            f.parentId = node.parentId;
            m_folders.append(f);

            folderStack.append(node);
            pendingFolderName.clear();
            continue;
        }

        // 匹配 DL 结束（退回上一层）
        if (line.contains("</DL>", Qt::CaseInsensitive) || line.contains("</DL", Qt::CaseInsensitive))
        {
            if (!folderStack.isEmpty())
                folderStack.removeLast();
            continue;
        }

        // 匹配链接: <DT><A HREF="URL" ...>标题</A>
        static QRegularExpression linkRe(R"(<A\s+HREF=\"([^\"]+)\"[^>]*>(.*?)</A>)",
                                          QRegularExpression::CaseInsensitiveOption);
        auto linkMatch = linkRe.match(line);
        if (linkMatch.hasMatch())
        {
            QString url = linkMatch.captured(1).trimmed();
            QString title = linkMatch.captured(2).trimmed();

            // 跳过 javascript: 和 place: 等特殊 URL
            if (url.startsWith("javascript:") || url.startsWith("place:"))
                continue;

            // 提取标签
            static QRegularExpression tagsRe(R"(TAGS=\"([^\"]*)\")", QRegularExpression::CaseInsensitiveOption);
            auto tagsMatch = tagsRe.match(line);

            Link link;
            link.url = url;
            link.title = title.isEmpty() ? url : title;
            link.folderId = folderStack.isEmpty() ? -1 : folderStack.last().id;

            if (tagsMatch.hasMatch())
            {
                link.description = tagsMatch.captured(1);   // 暂存到 description
            }

            m_links.append(link);
        }
    }
}
