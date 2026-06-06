// Application.cpp

#include "Application.h"
#include "DatabaseManager.h"
#include "impl/SqliteLinkRepository.h"
#include "impl/SqliteFolderRepository.h"
#include "impl/SqliteTagRepository.h"
#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

Application::Application() = default;
Application::~Application() = default;

bool Application::initialize()
{
    qInfo() << "[App] WebNav v1.0.0 \u6b63\u5728\u542f\u52a8...";

    auto &dbManager = DatabaseManager::instance();
    if (!dbManager.initialize())
    {
        qCritical() << "[App] \u6570\u636e\u5e93\u521d\u59cb\u5316\u5931\u8d25";
        return false;
    }

    auto &db = dbManager.database();
    auto linkRepo   = std::make_unique<SqliteLinkRepository>(db);
    auto folderRepo = std::make_unique<SqliteFolderRepository>(db);
    auto tagRepo    = std::make_unique<SqliteTagRepository>(db);

    applyTheme();
    setupApplicationSettings();

    m_mainWindow = std::make_unique<MainWindow>(
        linkRepo.release(), folderRepo.release(), tagRepo.release());
    m_mainWindow->show();
    return true;
}

void Application::applyTheme(const QString &themeName)
{
    QString qssFile = themeName.isEmpty()
        ? QStringLiteral(":/themes/light.qss")
        : QStringLiteral(":/themes/%1.qss").arg(themeName);

    QFile file(qssFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}

void Application::setupApplicationSettings() {}
