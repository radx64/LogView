#include "MainWindow.hpp"
#include "UiMainWindow.hpp"

#include <memory>

// TODO: cleanup this includes after some mockups creation and proper class segregation
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>

#include "AboutDialog.hpp"
#include "Bookmark.hpp"
#include "BookmarksModel.hpp"
#include "GrepDialogWindow.hpp"
#include "MergerDialogWindow.hpp"
#include "BookmarkDialogWindow.hpp"
#include "GrepNode.hpp"
#include "Logfile.hpp"
#include "ProjectModel.hpp"
#include "LogViewer.hpp"
#include "ChunkedTextView.hpp"
#include "LineSource.hpp"
#include "FileViewer.hpp"
#include "loader/Project.hpp"
#include "serializer/SerializerProjectModel.hpp"
#include "ProjectUiManager.hpp"
#include "ThemeManager.hpp"
#include "SettingsDialog.hpp"
#include "Settings.hpp"
#include "UpdateChecker.hpp"
#include "UpdateDialog.hpp"

#include <QTimer>

void MainWindow::closeFileTab(const int index)
{
    QTabWidget* tabWidget = ui->fileView;
    QWidget* tabContents = tabWidget->widget(index);
    tabWidget->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}
void MainWindow::showFileTabContextMenu(const QPoint& pos)
{
    QTabBar* bar = ui->fileView->tabBar();
    const int index = bar->tabAt(pos);
    if (index < 0)
        return;

    QMenu menu;
    QAction* copy_name_action = menu.addAction(tr("Copy file name"));
    QAction* copy_path_action = menu.addAction(tr("Copy file path"));
    QAction* open_folder_action = menu.addAction(tr("Open containing folder"));
    menu.addSeparator();
    QAction* reload_action = menu.addAction(tr("Reload"));
    menu.addSeparator();
    QAction* close_action = menu.addAction(tr("Close tab (MMB)"));

    const QString file_path = filePathForTab(index);
    const bool has_path = !file_path.isEmpty();
    copy_name_action->setEnabled(has_path);
    copy_path_action->setEnabled(has_path);
    open_folder_action->setEnabled(has_path);

    QAction* chosen = menu.exec(bar->mapToGlobal(pos));
    if (chosen == reload_action)
        pm_->reload_file(index);
    else if (chosen == close_action)
        closeFileTab(index);
    else if (chosen == copy_name_action)
        QApplication::clipboard()->setText(QFileInfo(file_path).fileName());
    else if (chosen == copy_path_action)
        QApplication::clipboard()->setText(QDir::toNativeSeparators(file_path));
    else if (chosen == open_folder_action)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file_path).absolutePath()));
}

QString MainWindow::filePathForTab(const int index) const
{
    FileViewer* viewer = dynamic_cast<FileViewer*>(ui->fileView->widget(index));
    if (viewer == nullptr || viewer->logfile_ == nullptr)
        return QString();
    return viewer->logfile_->getFileName();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->fileView->tabBar() && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::MiddleButton)
        {
            QTabBar* bar = ui->fileView->tabBar();
            const int index = bar->tabAt(mouse_event->pos());
            if (index >= 0)
            {
                closeFileTab(index);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::connect_signals()
{
    connect(ui->fileView, &QTabWidget::tabCloseRequested, this, &MainWindow::closeFileTab);

    QTabBar* bar = ui->fileView->tabBar();
    bar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bar, &QTabBar::customContextMenuRequested, this, &MainWindow::showFileTabContextMenu);
    bar->installEventFilter(this);

    connect(ui->fileView, &QTabWidget::currentChanged, this, &MainWindow::refreshWindowTitle);
}

void MainWindow::newProject()
{
    pm_->create_new();
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    ui->fileView->setTabsClosable(true);
    statusBar()->showMessage(tr("Use load from file menu or drop files in this window to begin."));
    connect_signals();
    setupThemeMenu();
    setupRecentMenus();
    pm_ = std::make_unique<ProjectUiManager>(ui);
    connect(pm_.get(), &ProjectUiManager::projectStateChanged, this, &MainWindow::project_changed);
    setupUpdateChecker();
    newProject();
    updateUi();
}

void MainWindow::setupUpdateChecker()
{
    update_checker_ = std::make_unique<UpdateChecker>(this);
    connect(update_checker_.get(), &UpdateChecker::updateAvailable,
            this, &MainWindow::onUpdateAvailable);
    connect(update_checker_.get(), &UpdateChecker::upToDate,
            this, &MainWindow::onUpToDate);
    connect(update_checker_.get(), &UpdateChecker::checkFailed,
            this, &MainWindow::onUpdateCheckFailed);

    if (Settings::instance().checkForUpdatesOnStartup())
    {
        // Defer slightly so the window is shown before any network work begins.
        QTimer::singleShot(2000, this, [this]() { checkForUpdates(/*silent=*/true); });
    }
}

void MainWindow::checkForUpdates(bool silent)
{
    manual_update_check_ = !silent;
    if (!silent)
        statusBar()->showMessage(tr("Checking for updates..."));
    update_checker_->checkForUpdates();
}

void MainWindow::on_actionCheck_for_updates_triggered()
{
    checkForUpdates(/*silent=*/false);
}

void MainWindow::onUpdateAvailable(const QString& version,
                                   const QString& url,
                                   const QString& title,
                                   const QString& notes)
{
    statusBar()->clearMessage();

    // During an automatic check, respect a version the user chose to skip.
    if (!manual_update_check_ &&
        version == Settings::instance().skippedUpdateVersion())
    {
        return;
    }

    UpdateDialog dialog(QString(APP_VERSION), version, url, title, notes, this);
    connect(&dialog, &UpdateDialog::skipVersionRequested, this,
            [](const QString& skipped)
            {
                Settings::instance().setSkippedUpdateVersion(skipped);
            });
    dialog.exec();
}

void MainWindow::onUpToDate(const QString& currentVersion)
{
    statusBar()->clearMessage();
    if (!manual_update_check_)
        return;

    QMessageBox::information(
        this, tr("Check for updates"),
        tr("You are running the latest version (%1).").arg(currentVersion));
}

void MainWindow::onUpdateCheckFailed(const QString& error)
{
    statusBar()->clearMessage();
    if (!manual_update_check_)
        return;

    QMessageBox::warning(
        this, tr("Check for updates"),
        tr("Could not check for updates:\n%1").arg(error));
}

void MainWindow::dropEvent(QDropEvent* event)
{
    event->acceptProposedAction();
    const QMimeData* mimeData = event->mimeData();
    if (!mimeData->hasUrls())
    {
        qDebug() << "Non URL mime data type";
        return;
    }

    QList<QUrl> urlList = mimeData->urls();
    for (const auto& fileList : urlList)
    {
        load_log_file(fileList.toLocalFile());
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  // if some actions should not be usable, like move, this code must be adopted
  event->acceptProposedAction();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::load_log_file(QString file_path)
{
   pm_->load_log_file(file_path);
   Settings::instance().addRecentFile(file_path);
}

FileViewer* MainWindow::get_active_viewer_widget()
{
    const int tab_index = ui->fileView->currentIndex();
    if(tab_index == -1) return nullptr;
    FileViewer* viewerWidget = dynamic_cast<FileViewer*>(ui->fileView->widget(tab_index));
    if (!viewerWidget) throw std::string("Could not find active ViewerWidget");
    return viewerWidget;
}

void MainWindow::grepCurrentView()
{
    //TODO make grep and bookmark active only when file is loaded
    FileViewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message

    LogViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    GrepDialogWindow grepDialog(this);

    if (grepDialog.exec() != QDialog::Accepted) return;
    auto result = grepDialog.getResult();
    if (deepest_tab)
    {
        GrepNode* new_grep_node = new GrepNode(result.pattern.toStdString(),
                                               result.is_regex,
                                               result.is_case_sensitive,
                                               result.is_inverted);

        deepest_tab->grep(new_grep_node);
        deepest_tab->getGrepNode()->addChild(new_grep_node);
    }
}

void MainWindow::bookmark_current_line()
{
    FileViewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message
    LogViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    if (!deepest_tab) return;
    const qint64 current_line_index = deepest_tab->text_->currentLine();
    const Line current_line = deepest_tab->source()->at(current_line_index);
    const uint32_t absolute_line_index = current_line.number;

    BookmarkDialogWindow dialog(this);
    dialog.setWindowTitle(tr("Add bookmark"));
    dialog.setName(current_line.text);
    dialog.setIcon(QString(":/icon/Gnome-Bookmark-New-32.png"));

    if (dialog.exec() != QDialog::Accepted) return;

    const auto result = dialog.getResult();
    viewerWidget->logfile_->getBookmarksModel()->add_bookmark(absolute_line_index,
        result.icon,
        result.name);
}

void MainWindow::on_actionLoad_from_file_triggered()
{
    QStringList file_paths = QFileDialog::getOpenFileNames(this,
        tr("Open log files"),
        Settings::instance().lastOpenedFileDirectory(),
        tr("All Files (*)"));
    if (file_paths.isEmpty())
        return;

    Settings::instance().setLastOpenedFileDirectory(
        QFileInfo(file_paths.constFirst()).absolutePath());

    for (const QString& file_path : file_paths)
        load_log_file(file_path);
}

void MainWindow::on_actionMerge_files_triggered()
{
    MergerDialogWindow dialog(this);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString merged_path = dialog.outputPath();
    if (merged_path.isEmpty())
        return;

    Settings::instance().addRecentFile(merged_path);

    if (dialog.shouldOpenResult())
        load_log_file(merged_path);
}

void MainWindow::on_actionGrep_current_view_triggered()
{
    grepCurrentView();
}

void MainWindow::on_actionExport_grep_triggered()
{
    FileViewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget)
    {
        QMessageBox::information(this, tr("Export grep"),
            tr("Open a file before exporting."));
        return;
    }

    LogViewer* deepest_tab = viewerWidget->getDeepestActiveTab();
    LineSource* source = deepest_tab ? deepest_tab->source() : nullptr;
    if (!source || source->count() <= 0)
    {
        QMessageBox::information(this, tr("Export grep"),
            tr("The active view has no content to export."));
        return;
    }

    const QString file_path = QFileDialog::getSaveFileName(this,
        tr("Export grep"),
        Settings::instance().lastOpenedFileDirectory(),
        tr("Text files (*.txt);;All Files (*)"));
    if (file_path.isEmpty())
        return;

    QFile out_file(file_path);
    if (!out_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::warning(this, tr("Export grep"),
            tr("Could not open %1 for writing.").arg(file_path));
        return;
    }

    Settings::instance().setLastOpenedFileDirectory(
        QFileInfo(file_path).absolutePath());

    QTextStream stream(&out_file);
    const qint64 total = source->count();
    for (qint64 row = 0; row < total; ++row)
        stream << source->at(row).text << '\n';
}
void MainWindow::on_actionBookmark_current_line_triggered()
{
    bookmark_current_line();
}

void MainWindow::on_exit_app_triggered()
{
    if (pm_->has_changed())
    {
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save changes you made in current project? All changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveProject();
    }   //duplicate code (move it to separate function later)

    QApplication::exit();
}

void MainWindow::on_actionAbout_triggered()
{
    const QString text = QString("Version ") + QString(APP_VERSION) +
        "\nBuild: " + __DATE__ + " " + __TIME__ +
        "\n\nradX64 © 2026\nReleased under\nGNU GENERAL PUBLIC LICENSE";

    AboutDialog dialog(text, this);
    dialog.exec();
}

void MainWindow::on_actionSave_project_as_triggered()
{
    if (pm_->is_empty())
    {
      QMessageBox::warning(this,"Warning!","Nothing to save.\nLoad file or project first.");
      return;
    }

    saveProject();
    updateUi();
}

void MainWindow::on_actionLoad_project_triggered()
{
    if (pm_->has_changed())
    {
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save changes you made in current project? All changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveProject();
    }

    openProject();
    updateUi();
}

void MainWindow::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle(title);
}

void MainWindow::refreshWindowTitle()
{
    QStringList parts;

    if (!pm_->project_name().isEmpty())
        parts << QFileInfo(pm_->project_name()).fileName() +
                 QString(pm_->has_changed()?" *":"");

    parts << "LogView " + QString(APP_VERSION);

    const int tab_index = ui->fileView->currentIndex();
    if (tab_index != -1)
    {
        FileViewer* viewer = dynamic_cast<FileViewer*>(ui->fileView->widget(tab_index));
        if (viewer && viewer->logfile_)
            parts << viewer->logfile_->getFileName();
    }

    setWindowTitle(parts.join(" - "));
}

void MainWindow::project_changed()
{
    updateUi();
}

void MainWindow::on_actionSave_project_triggered()
{
    pm_->save_project();
}

void MainWindow::updateMenus()
{
    ui->actionSave_project->setEnabled(!pm_->project_name().isEmpty() && pm_->has_changed());
}

void MainWindow::updateUi()
{
    refreshWindowTitle();
    updateMenus();
}

void MainWindow::setupThemeMenu()
{
    theme_manager_ = std::make_unique<ThemeManager>(this);

    theme_action_group_ = new QActionGroup(this);
    theme_action_group_->setExclusive(true);

    const auto registerThemeAction = [&](QAction* action, ThemeManager::Theme theme)
    {
        action->setData(static_cast<int>(theme));
        theme_action_group_->addAction(action);
        connect(action, &QAction::triggered, this, [this, theme]()
        {
            theme_manager_->setTheme(theme);
        });
    };

    registerThemeAction(ui->actionTheme_System, ThemeManager::Theme::System);
    registerThemeAction(ui->actionTheme_Light, ThemeManager::Theme::Light);
    registerThemeAction(ui->actionTheme_Dark, ThemeManager::Theme::Dark);

    connect(theme_manager_.get(), &ThemeManager::themeChanged,
            this, &MainWindow::updateThemeMenu);

    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::openSettings);

    theme_manager_->loadAndApply();
    updateThemeMenu();
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::setupRecentMenus()
{
    connect(&Settings::instance(), &Settings::changed,
            this, &MainWindow::updateRecentMenus);

    updateRecentMenus();
}

void MainWindow::updateRecentMenus()
{
    if (!ui->menuRecent_files || !ui->menuRecent_projects)
        return;

    const auto populate = [this](QMenu* menu, const QStringList& items, bool isProject)
    {
        menu->clear();
        if (items.isEmpty())
        {
            QAction* empty = menu->addAction(tr("(empty)"));
            empty->setEnabled(false);
            return;
        }

        int index = 1;
        for (const QString& path : items)
        {
            QAction* action =
                menu->addAction(QStringLiteral("&%1  %2").arg(index % 10).arg(path));
            action->setToolTip(path);
            if (isProject)
                connect(action, &QAction::triggered, this,
                        [this, path]() { openRecentProject(path); });
            else
                connect(action, &QAction::triggered, this,
                        [this, path]() { openRecentFile(path); });
            ++index;
        }
    };

    populate(ui->menuRecent_files, Settings::instance().recentFiles(), false);
    populate(ui->menuRecent_projects, Settings::instance().recentProjects(), true);
}

void MainWindow::openRecentFile(const QString& file_path)
{
    if (!QFileInfo::exists(file_path))
    {
        QMessageBox::warning(this, tr("Open recent file"),
            tr("File no longer exists:\n%1").arg(file_path));
        Settings::instance().removeRecentFile(file_path);
        return;
    }
    load_log_file(file_path);
}

void MainWindow::openRecentProject(const QString& file_path)
{
    if (!QFileInfo::exists(file_path))
    {
        QMessageBox::warning(this, tr("Open recent project"),
            tr("Project no longer exists:\n%1").arg(file_path));
        Settings::instance().removeRecentProject(file_path);
        return;
    }

    if (pm_->has_changed())
    {
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save changes you made in current project? All changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveProject();
    }

    const QString opened = pm_->open_project(file_path);
    if (!opened.isEmpty())
        Settings::instance().addRecentProject(opened);
    updateUi();
}

void MainWindow::updateThemeMenu()
{
    if (!theme_action_group_) return;
    const int current = static_cast<int>(theme_manager_->theme());
    for (QAction* action : theme_action_group_->actions())
    {
        if (action->data().toInt() == current)
        {
            action->setChecked(true);
            break;
        }
    }
}

void MainWindow::saveProject()
{
    pm_->save_project();
}
void MainWindow::openProject()
{
    const QString opened = pm_->open_project();
    if (!opened.isEmpty())
        Settings::instance().addRecentProject(opened);
}
