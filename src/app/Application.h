// Application.h — 应用主控类

#pragma once
#include <memory>
#include <QString>

class MainWindow;

class Application
{
public:
    Application();
    ~Application();

    bool initialize();
    MainWindow *mainWindow() const { return m_mainWindow.get(); }

private:
    void applyTheme(const QString &themeName = QString());
    void setupApplicationSettings();

    std::unique_ptr<MainWindow> m_mainWindow;
};
