#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

#include "Viewer.hpp"
#include "TabCompositeViewer.hpp"


class Logfile;
class QTextEdit;
class QTabWidget;
class ViewerWidget;

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

private:
    void bookmarkCurrentLine();
    void connect_signals();
    void grepCurrentView();
    void spawnViewerWithContent(const Logfile& log);
    ViewerWidget* get_active_viewer_widget();
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
