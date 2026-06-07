#pragma once
#include <memory>
#include <QObject>
#include <QString>

class QSystemTrayIcon;
class MainWindow;
class SqliteLinkRepository;
class SqliteFolderRepository;
class SqliteTagRepository;

class Application : public QObject
{
    Q_OBJECT
public:
    Application();
    ~Application() override;
    bool initialize();
    MainWindow *mainWindow() const { return m_mainWindow.get(); }

private:
    void applyTheme(const QString &themeName = QString());
    void setupTrayIcon();
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<SqliteLinkRepository> m_linkRepo;
    std::unique_ptr<SqliteFolderRepository> m_folderRepo;
    std::unique_ptr<SqliteTagRepository> m_tagRepo;
    QSystemTrayIcon *m_trayIcon = nullptr;
};
