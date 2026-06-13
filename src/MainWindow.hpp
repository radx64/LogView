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
class FileViewer;
class ThemeManager;
class QActionGroup;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionLoad_from_file_triggered();
    void closeFileTab(const int index);
    void showFileTabContextMenu(const QPoint& pos);
    void on_exit_app_triggered();
    void on_actionGrep_current_view_triggered();
    void on_actionBookmark_current_line_triggered();
    void on_actionAbout_triggered();
    void on_actionSave_project_as_triggered();
    void on_actionSave_project_triggered();
    void on_actionLoad_project_triggered();

private:
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

    void setupThemeMenu();
    void updateThemeMenu();
    void openSettings();

    std::unique_ptr<ProjectUiManager> pm_{};
    std::unique_ptr<ThemeManager> theme_manager_{};
    QActionGroup* theme_action_group_{nullptr};
    Ui::MainWindow *ui{nullptr};
};