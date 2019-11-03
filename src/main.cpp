#include "MainWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationVersion(APP_VERSION);
    MainWindow mainWindow;
    mainWindow.show();
    return QApplication::exec();
}
