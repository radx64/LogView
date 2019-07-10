#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QDrag>
#include <QHBoxLayout>
#include <QListWidget>

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
    std::vector<QTabWidget*> spawnedTabs_;
};

/* This Viewer class is only a POC to be replaced later by something more usefull*/
class Viewer : public QWidget
{
public:
    Viewer(QWidget* parent);
    QTextEdit* text_;

protected:
    QWidget* parent_;

};

class TabCompositeViewer : public Viewer
{
    Q_OBJECT
public:
    TabCompositeViewer(QWidget* parent);
    void grep(QString pattern);

    QStringList lines_;
    QTabWidget* tabs_;

public slots:
    void closeTab(const int);

protected:

};

class ViewerWidget: public QWidget
{
public:
    ViewerWidget(QWidget* parent);

    TabCompositeViewer* logViewer_;
protected:
    QWidget* parent_;
    QHBoxLayout* layout_;
    QListWidget* bookmarks_;
};



#endif // MAINWINDOW_H
