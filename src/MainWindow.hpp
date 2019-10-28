#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <memory>

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

#include "ProjectModelManager.hpp"

class Logfile;
class QTextEdit;
class QTabWidget;
class Viewer;

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

private:
    void bookmarkCurrentLine();
    void connect_signals();
    void grepCurrentView();
    void spawnViewerWithContent(QString file_path);
    Viewer* get_active_viewer_widget();
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);

    Ui::MainWindow *ui;
    ProjectModelManager manager_;

};

#endif // MAINWINDOW_HPP
