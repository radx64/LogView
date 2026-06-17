#include "UiMainWindow.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QCursor>
#include <QGridLayout>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMetaObject>
#include <QRect>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QWidget>

namespace Ui
{

void MainWindow::setupUi(QMainWindow* MainWindow)
{
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName("MainWindow");
    MainWindow->resize(631, 494);

    actionLoad_from_file = new QAction(MainWindow);
    actionLoad_from_file->setObjectName("actionLoad_from_file");
    QIcon icon;
    icon.addFile(QStringLiteral(":/icon/Gnome-Logviewer-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionLoad_from_file->setIcon(icon);
    actionLoad_from_file->setShortcutContext(Qt::ApplicationShortcut);

    exit_app = new QAction(MainWindow);
    exit_app->setObjectName("exit_app");
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/icon/Gnome-System-Log-Out-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    exit_app->setIcon(icon1);

    actionGrep_current_view = new QAction(MainWindow);
    actionGrep_current_view->setObjectName("actionGrep_current_view");
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icon/Gnome-Format-Indent-More-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionGrep_current_view->setIcon(icon2);

    actionBookmark_current_line = new QAction(MainWindow);
    actionBookmark_current_line->setObjectName("actionBookmark_current_line");
    QIcon icon3;
    icon3.addFile(QStringLiteral(":/icon/Gnome-Bookmark-New-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionBookmark_current_line->setIcon(icon3);

    actionAutomatic_markings = new QAction(MainWindow);
    actionAutomatic_markings->setObjectName("actionAutomatic_markings");

    actionAbout = new QAction(MainWindow);
    actionAbout->setObjectName("actionAbout");
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/icon/Gnome-Dialog-Warning-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionAbout->setIcon(icon4);

    actionCheck_for_updates = new QAction(MainWindow);
    actionCheck_for_updates->setObjectName("actionCheck_for_updates");

    actionLoad_project = new QAction(MainWindow);
    actionLoad_project->setObjectName("actionLoad_project");
    QIcon icon5;
    icon5.addFile(QStringLiteral(":/icon/Gnome-Document-Open-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionLoad_project->setIcon(icon5);

    actionSave_project_as = new QAction(MainWindow);
    actionSave_project_as->setObjectName("actionSave_project_as");
    QIcon icon6;
    icon6.addFile(QStringLiteral(":/icon/Gnome-Document-Save-As-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionSave_project_as->setIcon(icon6);

    actionSave_project = new QAction(MainWindow);
    actionSave_project->setObjectName("actionSave_project");
    QIcon icon7;
    icon7.addFile(QStringLiteral(":/icon/Gnome-Document-Save-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionSave_project->setIcon(icon7);

    centralWidget = new QWidget(MainWindow);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setAutoFillBackground(true);
    gridLayout_2 = new QGridLayout(centralWidget);
    gridLayout_2->setSpacing(6);
    gridLayout_2->setContentsMargins(11, 11, 11, 11);
    gridLayout_2->setObjectName("gridLayout_2");
    gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setObjectName("gridLayout");
    fileView = new QTabWidget(centralWidget);
    fileView->setObjectName("fileView");
    fileView->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
    fileView->setAutoFillBackground(true);
    fileView->setTabShape(QTabWidget::Rounded);

    gridLayout->addWidget(fileView, 0, 0, 1, 1);

    gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

    MainWindow->setCentralWidget(centralWidget);
    menuBar = new QMenuBar(MainWindow);
    menuBar->setObjectName("menuBar");
    menuBar->setGeometry(QRect(0, 0, 631, 20));
    menuTest = new QMenu(menuBar);
    menuTest->setObjectName("menuTest");
    menuFind = new QMenu(menuBar);
    menuFind->setObjectName("menuFind");
    menuHelp = new QMenu(menuBar);
    menuHelp->setObjectName("menuHelp");
    menuHelp->setLayoutDirection(Qt::LeftToRight);
    MainWindow->setMenuBar(menuBar);
    mainToolBar = new QToolBar(MainWindow);
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->setMouseTracking(true);
    mainToolBar->setAutoFillBackground(false);
    MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
    statusBar = new QStatusBar(MainWindow);
    statusBar->setObjectName("statusBar");
    MainWindow->setStatusBar(statusBar);

    menuBar->addAction(menuTest->menuAction());
    menuBar->addAction(menuFind->menuAction());
    menuBar->addAction(menuHelp->menuAction());
    menuTest->addSeparator();
    menuTest->addAction(actionLoad_from_file);
    menuTest->addSeparator();
    menuTest->addAction(actionLoad_project);
    menuTest->addAction(actionSave_project);
    menuTest->addAction(actionSave_project_as);
    menuTest->addSeparator();
    menuTest->addAction(exit_app);
    menuTest->addSeparator();
    menuFind->addAction(actionGrep_current_view);
    menuFind->addAction(actionBookmark_current_line);
    menuFind->addSeparator();
    menuFind->addAction(actionAutomatic_markings);
    menuHelp->addAction(actionCheck_for_updates);
    menuHelp->addSeparator();
    menuHelp->addAction(actionAbout);
    mainToolBar->addAction(actionLoad_from_file);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionLoad_project);
    mainToolBar->addAction(actionSave_project);
    mainToolBar->addAction(actionSave_project_as);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionBookmark_current_line);
    mainToolBar->addAction(actionGrep_current_view);

    retranslateUi(MainWindow);

    fileView->setCurrentIndex(-1);

    QMetaObject::connectSlotsByName(MainWindow);
}

void MainWindow::retranslateUi(QMainWindow* MainWindow)
{
    MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LogView", nullptr));
    actionLoad_from_file->setText(QCoreApplication::translate("MainWindow", "Open file...", nullptr));
    actionLoad_from_file->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
    exit_app->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
    exit_app->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
    actionGrep_current_view->setText(QCoreApplication::translate("MainWindow", "Grep current view", nullptr));
    actionGrep_current_view->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+G", nullptr));
    actionBookmark_current_line->setText(QCoreApplication::translate("MainWindow", "Bookmark current line", nullptr));
    actionBookmark_current_line->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+B", nullptr));
    actionAutomatic_markings->setText(QCoreApplication::translate("MainWindow", "Automatic markings...", nullptr));
    actionAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
    actionCheck_for_updates->setText(QCoreApplication::translate("MainWindow", "Check for updates...", nullptr));
    actionLoad_project->setText(QCoreApplication::translate("MainWindow", "Load project ...", nullptr));
    actionLoad_project->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+L", nullptr));
    actionSave_project_as->setText(QCoreApplication::translate("MainWindow", "Save project as...", nullptr));
    actionSave_project_as->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S", nullptr));
    actionSave_project->setText(QCoreApplication::translate("MainWindow", "Save project", nullptr));
    actionSave_project->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
    menuTest->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
    menuFind->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
    menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
    mainToolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "Toolbar", nullptr));
}

} // namespace Ui
