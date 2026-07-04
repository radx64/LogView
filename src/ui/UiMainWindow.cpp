#include "UiMainWindow.hpp"

#include "Translator.hpp"

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

    actionMerge_files = new QAction(MainWindow);
    actionMerge_files->setObjectName("actionMerge_files");
    QIcon iconMerge;
    iconMerge.addFile(QStringLiteral(":/icon/Add-Files-To-Archive-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionMerge_files->setIcon(iconMerge);

    exit_app = new QAction(MainWindow);
    exit_app->setObjectName("exit_app");
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/icon/Gnome-System-Log-Out-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    exit_app->setIcon(icon1);

    actionFind = new QAction(MainWindow);
    actionFind->setObjectName("actionFind");
    QIcon iconFind;
    iconFind.addFile(QStringLiteral(":/icon/Gnome-Edit-Find-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionFind->setIcon(iconFind);

    actionGrep_current_view = new QAction(MainWindow);
    actionGrep_current_view->setObjectName("actionGrep_current_view");
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icon/Gnome-Format-Indent-More-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionGrep_current_view->setIcon(icon2);

    actionExport_grep = new QAction(MainWindow);
    actionExport_grep->setObjectName("actionExport_grep");
    QIcon icon3;
    icon3.addFile(QStringLiteral(":/icon/Gnome-Document-Send-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionExport_grep->setIcon(icon3);

    actionBookmark_current_line = new QAction(MainWindow);
    actionBookmark_current_line->setObjectName("actionBookmark_current_line");
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/icon/Gnome-Bookmark-New-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionBookmark_current_line->setIcon(icon4);

    actionAbout = new QAction(MainWindow);
    actionAbout->setObjectName("actionAbout");
    QIcon icon5;
    icon5.addFile(QStringLiteral(":/icon/Gnome-Dialog-Warning-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionAbout->setIcon(icon5);

    actionCheck_for_updates = new QAction(MainWindow);
    actionCheck_for_updates->setObjectName("actionCheck_for_updates");

    actionLoad_project = new QAction(MainWindow);
    actionLoad_project->setObjectName("actionLoad_project");
    QIcon icon6;
    icon6.addFile(QStringLiteral(":/icon/Gnome-Document-Open-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionLoad_project->setIcon(icon6);

    actionSave_project_as = new QAction(MainWindow);
    actionSave_project_as->setObjectName("actionSave_project_as");
    QIcon icon7;
    icon7.addFile(QStringLiteral(":/icon/Gnome-Document-Save-As-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionSave_project_as->setIcon(icon7);

    actionSave_project = new QAction(MainWindow);
    actionSave_project->setObjectName("actionSave_project");
    QIcon icon8;
    icon8.addFile(QStringLiteral(":/icon/Gnome-Document-Save-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionSave_project->setIcon(icon8);

    actionTheme_System = new QAction(MainWindow);
    actionTheme_System->setObjectName("actionTheme_System");
    actionTheme_System->setCheckable(true);

    actionTheme_Light = new QAction(MainWindow);
    actionTheme_Light->setObjectName("actionTheme_Light");
    actionTheme_Light->setCheckable(true);

    actionTheme_Dark = new QAction(MainWindow);
    actionTheme_Dark->setObjectName("actionTheme_Dark");
    actionTheme_Dark->setCheckable(true);

    actionOptions = new QAction(MainWindow);
    actionOptions->setObjectName("actionOptions");
    QIcon icon9;
    icon9.addFile(QStringLiteral(":/icon/Gnome-Preferences-System-32.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    actionOptions->setIcon(icon9);

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
    menuRecent_files = new QMenu(menuTest);
    menuRecent_files->setObjectName("menuRecent_files");
    menuRecent_files->setToolTipsVisible(true);
    menuRecent_projects = new QMenu(menuTest);
    menuRecent_projects->setObjectName("menuRecent_projects");
    menuRecent_projects->setToolTipsVisible(true);
    menuFind = new QMenu(menuBar);
    menuFind->setObjectName("menuFind");
    menuView = new QMenu(menuBar);
    menuView->setObjectName("menuView");
    menuTheme = new QMenu(menuView);
    menuTheme->setObjectName("menuTheme");
    menuTheme->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Preferences-Desktop-Theme-32.png")));
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
    menuBar->addAction(menuView->menuAction());
    menuBar->addAction(menuHelp->menuAction());
    menuTest->addSeparator();
    menuTest->addAction(actionLoad_from_file);
    menuTest->addAction(menuRecent_files->menuAction());
    menuTest->addSeparator();
    menuTest->addAction(actionMerge_files);
    menuTest->addSeparator();
    menuTest->addAction(actionLoad_project);
    menuTest->addAction(actionSave_project);
    menuTest->addAction(actionSave_project_as);
    menuTest->addAction(menuRecent_projects->menuAction());
    menuTest->addSeparator();
    menuTest->addAction(actionExport_grep);
    menuTest->addSeparator();
    menuTest->addAction(exit_app);
    menuTest->addSeparator();
    menuFind->addAction(actionFind);
    menuFind->addSeparator();
    menuFind->addAction(actionGrep_current_view);
    menuFind->addAction(actionBookmark_current_line);
    menuView->addAction(menuTheme->menuAction());
    menuTheme->addAction(actionTheme_System);
    menuTheme->addAction(actionTheme_Light);
    menuTheme->addAction(actionTheme_Dark);
    menuView->addSeparator();
    menuView->addAction(actionOptions);
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
    MainWindow->setWindowTitle(Lang::tr("app.title"));

    actionLoad_from_file->setText(Lang::tr("action.open_file"));
    actionLoad_from_file->setShortcut(QStringLiteral("Ctrl+O"));

    actionMerge_files->setText(Lang::tr("action.merge_files"));

    exit_app->setText(Lang::tr("action.exit"));
    exit_app->setShortcut(QStringLiteral("Ctrl+Q"));

    actionFind->setText(Lang::tr("action.find"));
    actionFind->setShortcut(QStringLiteral("Ctrl+F"));

    actionGrep_current_view->setText(Lang::tr("action.grep_current_view"));
    actionGrep_current_view->setShortcut(QStringLiteral("Ctrl+G"));

    actionExport_grep->setText(Lang::tr("action.export_grep"));
    actionExport_grep->setShortcut(QStringLiteral("Ctrl+E"));

    actionBookmark_current_line->setText(Lang::tr("action.bookmark_line"));
    actionBookmark_current_line->setShortcut(QStringLiteral("Ctrl+B"));

    actionAbout->setText(Lang::tr("action.about"));

    actionCheck_for_updates->setText(Lang::tr("action.check_updates"));

    actionLoad_project->setText(Lang::tr("action.load_project"));
    actionLoad_project->setShortcut(QStringLiteral("Ctrl+L"));

    actionSave_project_as->setText(Lang::tr("action.save_project_as"));
    actionSave_project_as->setShortcut(QStringLiteral("Ctrl+Shift+S"));

    actionSave_project->setText(Lang::tr("action.save_project"));
    actionSave_project->setShortcut(QStringLiteral("Ctrl+S"));

    actionTheme_System->setText(Lang::tr("theme.system"));
    actionTheme_Light->setText(Lang::tr("theme.light"));
    actionTheme_Dark->setText(Lang::tr("theme.dark"));
    actionOptions->setText(Lang::tr("action.options"));

    menuTest->setTitle(Lang::tr("menu.file"));
    menuFind->setTitle(Lang::tr("menu.edit"));
    menuView->setTitle(Lang::tr("menu.view"));
    menuTheme->setTitle(Lang::tr("menu.theme"));
    menuRecent_files->setTitle(Lang::tr("menu.recent_files"));
    menuRecent_projects->setTitle(Lang::tr("menu.recent_projects"));
    menuHelp->setTitle(Lang::tr("menu.help"));
    mainToolBar->setWindowTitle(Lang::tr("toolbar.title"));
}

} // namespace Ui
