// main.cpp — WebNav 应用入口
#include <QApplication>
#include <QIcon>
#include "app/Application.h"

int main(int argc, char *argv[])
{
    QApplication qtApp(argc, argv);
    QApplication::setApplicationName("WebNav");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("WebNav");
    QApplication::setWindowIcon(QIcon(":/icons/app.svg"));

    Application navApp;
    navApp.initialize();

    return QApplication::exec();
}
