#include "Application.h"
#include "database/DatabaseManager.h"
#include "impl/SqliteLinkRepository.h"
#include "impl/SqliteFolderRepository.h"
#include "impl/SqliteTagRepository.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QDebug>

Application::Application() = default;

Application::~Application() = default;

bool Application::initialize()
{
    qInfo() << "[App] WebNav v1.0.0 启动...";
    auto &dbManager = DatabaseManager::instance();
    if (!dbManager.initialize()) {
        qCritical() << "[App] 数据库初始化失败";
        return false;
    }
    auto &db = dbManager.database();
    m_linkRepo = std::make_unique<SqliteLinkRepository>(db);
    m_folderRepo = std::make_unique<SqliteFolderRepository>(db);
    m_tagRepo = std::make_unique<SqliteTagRepository>(db);
    // 从 QSettings 加载保存的主题设置，默认深色
    QSettings settings;
    QString themeName = settings.value("theme", "dark").toString();
    applyTheme(themeName);
    m_mainWindow = std::make_unique<MainWindow>(
        m_linkRepo.get(), m_folderRepo.get(), m_tagRepo.get());
    m_mainWindow->show();
    qInfo() << "[App] 启动完成";
    return true;
}

void Application::applyTheme(const QString &themeName)
{
    QString qssFile;
    if (themeName == "dark")
        qssFile = QStringLiteral(":/themes/dark.qss");
    else
        qssFile = QStringLiteral(":/themes/light.qss");
    QFile file(qssFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}
