#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>

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

private:
    void spawnEditorWithContent(QString& name, QString& content);

    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);

    Ui::MainWindow *ui;
    std::vector<QTabWidget*> spawnedTabs_;
};


class Viewer : public QWidget
{
public:
    Viewer(QWidget* parent);

protected:
    QWidget* parent_;
    QTextEdit* text_;
};

class TabCompositeViewer : public Viewer
{
    Q_OBJECT
public:
    TabCompositeViewer(QWidget* parent);
    void grep();

public slots:
    void closeTab(const int);

protected:
    Viewer* viewer_;
    QTabWidget* tabs_;
};

#endif // MAINWINDOW_H
