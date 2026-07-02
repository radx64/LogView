#include "MainWindow.hpp"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("LogView");
    QApplication::setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "Log file viewer."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
        QCoreApplication::translate("main", "files"),
        QCoreApplication::translate("main",
            "Log file(s) or project (.json) to open."),
        QCoreApplication::translate("main", "[files...]"));
    parser.process(a);

    MainWindow mainWindow;
    mainWindow.setMinimumSize(200,200);
    mainWindow.show();

    const QStringList files = parser.positionalArguments();
    if (!files.isEmpty())
        mainWindow.openCommandLinePaths(files);

    return QApplication::exec();
}
