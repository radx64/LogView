#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

#include "Viewer.hpp"
#include "TabCompositeViewer.hpp"

class QTextEdit;
class QTabWidget;

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
    void grepCurrentView();
    void bookmarkCurrentLine();
    void closeFileTab(const int index);

private:
    void spawnViewerWithContent(QString& name, QStringList& content);

    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
