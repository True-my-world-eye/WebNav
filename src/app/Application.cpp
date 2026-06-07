#include "Application.h"
#include "database/DatabaseManager.h"
#include "impl/SqliteLinkRepository.h"
#include "impl/SqliteFolderRepository.h"
#include "impl/SqliteTagRepository.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
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

    // ── 系统托盘图标 ──
    setupTrayIcon();

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

void Application::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;

    m_trayIcon = new QSystemTrayIcon(QIcon(":/icons/app.svg"), this);
    m_trayIcon->setToolTip(QStringLiteral("WebNav - 链接管理器"));

    auto *trayMenu = new QMenu();
    auto *showAct = trayMenu->addAction(QStringLiteral("显示窗口"));
    connect(showAct, &QAction::triggered, this, [this]() {
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
    });
    auto *hideAct = trayMenu->addAction(QStringLiteral("隐藏到托盘"));
    connect(hideAct, &QAction::triggered, this, [this]() {
        m_mainWindow->hide();
    });
    trayMenu->addSeparator();
    auto *quitAct = trayMenu->addAction(QStringLiteral("退出"));
    connect(quitAct, &QAction::triggered, this, [this]() {
        qApp->quit();
    });

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();

    // 双击托盘恢复窗口
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            m_mainWindow->show();
            m_mainWindow->raise();
            m_mainWindow->activateWindow();
        }
    });
}
