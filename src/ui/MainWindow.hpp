#pragma once

#include <memory>

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

#include "ProjectUiManager.hpp"

class Logfile;
class QTextEdit;
class QTabWidget;
class QMenu;
class FileViewer;
class ThemeManager;
class QActionGroup;
class UpdateChecker;
class QProgressBar;
class QLabel;
class QCloseEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openCommandLinePaths(const QStringList& paths);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_actionLoad_from_file_triggered();
    void on_actionMerge_files_triggered();
    void closeFileTab(const int index);
    void showFileTabContextMenu(const QPoint& pos);
    void on_exit_app_triggered();
    void on_actionFind_triggered();
    void on_actionGrep_current_view_triggered();
    void on_actionExport_grep_triggered();
    void on_actionBookmark_current_line_triggered();
    void on_actionAbout_triggered();
    void on_actionSave_project_as_triggered();
    void on_actionSave_project_triggered();
    void on_actionLoad_project_triggered();
    void on_actionCheck_for_updates_triggered();

private:
    void setupLoadProgressUi();
    void onLoadProgressChanged(bool active, int percent, const QString& text);

    void setupUpdateChecker();
    void checkForUpdates(bool silent);
    void onUpdateAvailable(const QString& version,
                           const QString& url,
                           const QString& title,
                           const QString& notes);
    void onUpToDate(const QString& currentVersion);
    void onUpdateCheckFailed(const QString& error);

    QString filePathForTab(const int index) const;
    void project_changed();
    void bookmark_current_line();
    void connect_signals();
    void grepCurrentView();
    void load_log_file(QString file_path);
    FileViewer* get_active_viewer_widget();
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void setWindowTitle(const QString& title);
    void updateUi();
    void updateMenus();
    void refreshWindowTitle();
    void newProject();
    void saveProject();
    void openProject();
    bool confirmDiscardChanges();

    void setupThemeMenu();
    void updateThemeMenu();
    void openSettings();

    void setupRecentMenus();
    void updateRecentMenus();
    void openRecentFile(const QString& file_path);
    void openRecentProject(const QString& file_path);

    std::unique_ptr<ProjectUiManager> pm_{};
    std::unique_ptr<ThemeManager> theme_manager_{};
    std::unique_ptr<UpdateChecker> update_checker_{};
    QActionGroup* theme_action_group_{nullptr};
    bool manual_update_check_{false};
    QProgressBar* load_progress_bar_{nullptr};
    QLabel* load_progress_label_{nullptr};
    Ui::MainWindow *ui{nullptr};
};