// PlatformUtils.h — 跨平台工具函数

#pragma once
#include <QString>
#include <QWidget>

class PlatformUtils
{
public:
    // 在默认浏览器中打开链接
    static bool openInBrowser(const QString &url);

    // 在指定浏览器中打开链接
    static bool openInBrowser(const QString &url, const QString &browserPath);

    // 获取默认浏览器路径
    static QString defaultBrowserPath();

    // 获取支持的浏览器列表（路径 + 名称）
    struct BrowserInfo {
        QString name;
        QString path;
    };
    static QVector<BrowserInfo> detectedBrowsers();

    // 显示系统通知
    static bool showNotification(const QString &title, const QString &message);

    // 获取应用数据目录
    static QString appDataDirectory();
};
