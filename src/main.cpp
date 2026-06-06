// main.cpp — WebNav 应用入口
// 基于 Qt 6 的桌面网页链接管理器
#include <QApplication>
#include "app/Application.h"

int main(int argc, char *argv[])
{
    QApplication qApp(argc, argv);
    QApplication::setApplicationName("WebNav");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("WebNav");

    Application app;        // 应用初始化：数据库、主题、托盘等
    app.initialize();

    return QApplication::exec();
}
