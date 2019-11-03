#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

class Logfile;
class QTextEdit;
class QTabWidget;
class ProjectViewer;
class ProjectModel;

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
    void on_exit_app_triggered();
    void on_actionGrep_current_view_triggered();
    void on_actionBookmark_current_line_triggered();
    void on_actionAbout_triggered();
    void on_actionSave_project_triggered();
    void on_actionLoad_project_triggered();
    void project_changed();

private:
    void bookmarkCurrentLine();
    void connect_signals();
    void grepCurrentView();
    void spawnViewerWithContent(QString file_path);
    ProjectViewer* get_active_viewer_widget();
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void setWindowTitle(const QString& title);
    void refreshWindowTitle();

    Ui::MainWindow *ui{nullptr};
    ProjectModel *pm_{nullptr};
};

#endif // MAINWINDOW_HPP
