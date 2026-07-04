#include "MainWindow.hpp"
#include "Settings.hpp"
#include "Translator.hpp"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("LogView");
    QApplication::setApplicationVersion(APP_VERSION);

    Translator::instance().setLanguage(Settings::instance().language());

    QCommandLineParser parser;
    parser.setApplicationDescription(Lang::tr("cli.description"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(
        Lang::tr("cli.files_arg"),
        Lang::tr("cli.files_desc"),
        QStringLiteral("[files...]"));
    parser.process(a);

    MainWindow mainWindow;
    mainWindow.setMinimumSize(200,200);
    mainWindow.show();

    const QStringList files = parser.positionalArguments();
    if (!files.isEmpty())
        mainWindow.openCommandLinePaths(files);

    return QApplication::exec();
}
