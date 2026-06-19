#pragma once

#include <QAction>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QWidget>

namespace Ui
{

class MainWindow
{
public:
    QAction* actionLoad_from_file{nullptr};
    QAction* exit_app{nullptr};
    QAction* actionGrep_current_view{nullptr};
    QAction* actionExport_grep{nullptr};
    QAction* actionBookmark_current_line{nullptr};
    QAction* actionAbout{nullptr};
    QAction* actionCheck_for_updates{nullptr};
    QAction* actionLoad_project{nullptr};
    QAction* actionSave_project_as{nullptr};
    QAction* actionSave_project{nullptr};
    QWidget* centralWidget{nullptr};
    QGridLayout* gridLayout_2{nullptr};
    QGridLayout* gridLayout{nullptr};
    QTabWidget* fileView{nullptr};
    QMenuBar* menuBar{nullptr};
    QMenu* menuTest{nullptr};
    QMenu* menuFind{nullptr};
    QMenu* menuHelp{nullptr};
    QToolBar* mainToolBar{nullptr};
    QStatusBar* statusBar{nullptr};

    void setupUi(QMainWindow* MainWindow);
    void retranslateUi(QMainWindow* MainWindow);
};

} // namespace Ui
