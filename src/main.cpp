#include "MainWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion(APP_VERSION);
    MainWindow mainWindow;
    mainWindow.show();
    return a.exec();
}
