#include "PlatformUtils.h"
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

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
    return {};
}

QVector<PlatformUtils::BrowserInfo> PlatformUtils::detectedBrowsers()
{
    QVector<BrowserInfo> browsers;
#ifdef Q_OS_WIN
    auto checkBrowser = [&](const QString &name, const QString &path) {
        if (QFile::exists(path))
            browsers.append({name, path});
    };
    QString programFiles = qEnvironmentVariable("ProgramFiles");
    QString programFilesX86 = qEnvironmentVariable("ProgramFiles(x86)");
    checkBrowser("Chrome",  programFiles + "/Google/Chrome/Application/chrome.exe");
    checkBrowser("Edge",    programFilesX86 + "/Microsoft/Edge/Application/msedge.exe");
    checkBrowser("Firefox", programFiles + "/Mozilla Firefox/firefox.exe");
#endif
    return browsers;
}

bool PlatformUtils::showNotification(const QString &title, const QString &message)
{
    Q_UNUSED(title) Q_UNUSED(message)
    return false;
}

QString PlatformUtils::appDataDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}
