// BookmarkExporter.cpp — 书签导出服务实现

#include "BookmarkExporter.h"
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QDateTime>
#include <functional>

bool BookmarkExporter::exportToFile(const QString &filePath,
                                     const QVector<Link> &links,
                                     const QVector<Folder> &folders)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "[Exporter] 无法写入文件:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "<!DOCTYPE NETSCAPE-Bookmark-file-1>\n";
    out << "<!-- This is an automatically generated file. -->\n";
    out << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n";
    out << "<TITLE>Bookmarks</TITLE>\n";
    out << "<H1>WebNav \u4e66\u7b7e\u5bfc\u51fa</H1>\n";
    out << "<DL><p>\n";

    // 按文件夹分组
    QMap<int, QVector<Link>> folderLinks;
    QVector<Link> rootLinks;
    for (const auto &link : links)
    {
        if (link.folderId > 0)
            folderLinks[link.folderId].append(link);
        else
            rootLinks.append(link);
    }

    QMap<int, Folder> folderMap;
    for (const auto &f : folders)
        folderMap[f.id] = f;

    // 递归导出文件夹结构
    std::function<void(int, int)> exportFolder;
    exportFolder = [&](int parentId, int depth) {
        QString indent = QString("    ").repeated(depth + 1);

        for (const auto &f : folders)
        {
            if (f.parentId != parentId) continue;

            out << indent << "<DT><H3>" << f.name.toHtmlEscaped() << "</H3>\n";
            out << indent << "<DL><p>\n";

            if (folderLinks.contains(f.id))
            {
                for (const auto &link : folderLinks[f.id])
                {
                    QString addDate = QString::number(QDateTime::currentSecsSinceEpoch());
                    out << indent << "    <DT><A HREF=\"" << link.url.toHtmlEscaped()
                        << "\" ADD_DATE=\"" << addDate << "\">"
                        << link.title.toHtmlEscaped() << "</A>\n";
                }
            }

            exportFolder(f.id, depth + 1);
            out << indent << "</DL><p>\n";
        }
    };

    // 根级链接
    for (const auto &link : rootLinks)
    {
        QString addDate = QString::number(QDateTime::currentSecsSinceEpoch());
        out << "    <DT><A HREF=\"" << link.url.toHtmlEscaped()
            << "\" ADD_DATE=\"" << addDate << "\">"
            << link.title.toHtmlEscaped() << "</A>\n";
    }

    exportFolder(-1, 1);
    out << "</DL><p>\n";
    file.close();
    return true;
}

bool BookmarkExporter::exportToMarkdown(const QString &filePath,
                                         const QVector<Link> &links,
                                         const QVector<Folder> &folders)
{
    Q_UNUSED(folders)

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "[Exporter] 无法写入 Markdown:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "# WebNav \u4e66\u7b7e\u5bfc\u51fa\n\n";
    out << "> \u5bfc\u51fa\u65f6\u95f4: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";
    out << "---\n\n";

    for (const auto &link : links)
    {
        out << "- [" << link.title << "](" << link.url << ")\n";
        if (!link.description.isEmpty())
            out << "  - " << link.description << "\n";
    }

    file.close();
    return true;
}
