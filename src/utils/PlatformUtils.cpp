// PlatformUtils.cpp

#include "PlatformUtils.h"
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>

bool PlatformUtils::openInBrowser(const QString &url)
{
    return QDesktopServices::openUrl(QUrl(url));
}

bool PlatformUtils::openInBrowser(const QString &url, const QString &browserPath)
{
    return QProcess::startDetached(browserPath, {url});
}

QString PlatformUtils::defaultBrowserPath()
{
#ifdef Q_OS_WIN
    // Windows 默认浏览器注册表路径
    return QString();
#else
    return QString();
#endif
}

QVector<PlatformUtils::BrowserInfo> PlatformUtils::detectedBrowsers()
{
    QVector<BrowserInfo> browsers;

#ifdef Q_OS_WIN
    // 检测常见浏览器
    auto checkBrowser = [&](const QString &name, const QString &path) {
        if (QFile::exists(path))
            browsers.append({name, path});
    };

    QString programFiles = qEnvironmentVariable("ProgramFiles");
    QString programFilesX86 = qEnvironmentVariable("ProgramFiles(x86)");
    QString localAppData = qEnvironmentVariable("LOCALAPPDATA");

    checkBrowser(QStringLiteral("Chrome"),  programFiles + "/Google/Chrome/Application/chrome.exe");
    checkBrowser(QStringLiteral("Edge"),    programFilesX86 + "/Microsoft/Edge/Application/msedge.exe");
    checkBrowser(QStringLiteral("Firefox"), programFiles + "/Mozilla Firefox/firefox.exe");
#endif

    return browsers;
}

bool PlatformUtils::showNotification(const QString &title, const QString &message)
{
    Q_UNUSED(title)
    Q_UNUSED(message)
    // TODO: Phase 2 使用 QSystemTrayIcon::showMessage
    return false;
}

QString PlatformUtils::appDataDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
