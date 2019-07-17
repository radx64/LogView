#include "MainWindow.hpp"
#include "ui_MainWindow.h"

// TODO: cleanup this includes after some mockups creation and proper class segregation
#include <QLabel>
#include <QLayout>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QAction>
#include <QMimeData>
#include <QInputDialog>
#include <QDebug>
#include <QStandardItemModel>

#include "Bookmark.hpp"
#include "BookmarksModel.hpp"
#include "Logfile.hpp"
#include "Viewer.hpp"
#include "ViewerWidget.hpp"
#include "TabCompositeViewer.hpp"
#include "TextRenderer.hpp"

void MainWindow::closeFileTab(const int index)
{
    QTabWidget* tabWidget = ui->fileView;
    QWidget* tabContents = tabWidget->widget(index);
    tabWidget->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}
void MainWindow::connect_signals()
{
    connect(ui->fileView, &QTabWidget::tabCloseRequested, this, &MainWindow::closeFileTab);
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
}

void MainWindow::dropEvent(QDropEvent* event)
{
    qDebug() << "Something was dropped here";
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
        spawnViewerWithContent(Logfile(fileList.toLocalFile()));
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

void MainWindow::spawnViewerWithContent(const Logfile& log)
{
    QTabWidget* fileTabWidget = ui->fileView;
    ViewerWidget* viewer = new ViewerWidget(fileTabWidget);
    fileTabWidget ->addTab(viewer, log.getFileName().split(QRegularExpression("[\\/]")).last());
    Lines content = log.getLines();
    viewer->logViewer_->setContent(content);
}

TabCompositeViewer* find_deepest_active_tab(TabCompositeViewer* start_point)
{
    if (start_point == nullptr) return start_point;
    const int tab_grep_index = start_point->tabs_->currentIndex();
    qDebug() << "tab_grep_index:" << tab_grep_index;
    QWidget* active_tab = start_point->tabs_->widget(tab_grep_index);
    TabCompositeViewer* active_tab_casted = dynamic_cast<TabCompositeViewer*>(active_tab);
    if (active_tab_casted == nullptr) return start_point;
    TabCompositeViewer* result = find_deepest_active_tab(active_tab_casted);
    return result ? result : start_point;
}

ViewerWidget* MainWindow::get_active_viewer_widget()
{
    const int tab_index = ui->fileView->currentIndex();
    qDebug() << "File tab index:" <<tab_index;
    ViewerWidget* viewerWidget = dynamic_cast<ViewerWidget*>(ui->fileView->widget(tab_index));
    if (!viewerWidget) throw std::string("Could not find active ViewerWidget");
    return viewerWidget;
}

void MainWindow::grepCurrentView()
{
    /*
     * This is totally experimental approach with recursive components search
     * It works but I hate it due those dynamic casts
     * Need to change approach to create model of greps and then render it recursively
     */

    ViewerWidget* viewerWidget = get_active_viewer_widget();
    QWidget* deepest_tab = find_deepest_active_tab(viewerWidget->logViewer_);
    TabCompositeViewer* deepest_tab_casted = dynamic_cast<TabCompositeViewer*>(deepest_tab);

    // Simple QInputDialog will be extended later for something more fancy
    bool ok = false;
    QString input_grep = QInputDialog::getText(this, tr("Grepping..."),
        tr("grep:"), QLineEdit::Normal, "", &ok);

    if (deepest_tab_casted && ok) deepest_tab_casted->grep(input_grep);
}

void MainWindow::bookmarkCurrentLine()
{
    qDebug() << "Would normally bookmark current line";

    ViewerWidget* viewerWidget = get_active_viewer_widget();
    QWidget* deepest_tab = find_deepest_active_tab(viewerWidget->logViewer_);
    TabCompositeViewer* deepest_tab_casted = dynamic_cast<TabCompositeViewer*>(deepest_tab);


    if (!deepest_tab_casted) return;
    int current_line_index = deepest_tab_casted->text_ ->textCursor().blockNumber();
    uint32_t absolute_line_index = deepest_tab_casted->lines_[current_line_index].number;

    // Simple QInputDialog will be extended later for something more fancy
    bool ok = false;
    QString bookmark_name = QInputDialog::getText(this, tr("Bookmark creation"),
        tr("Name:"), QLineEdit::Normal, deepest_tab_casted->lines_[current_line_index].text, &ok);

    if (!ok) return;
    qDebug() << "Adding bookmark at line" << absolute_line_index;
    viewerWidget->bookmarks_model_->add_bookmark(absolute_line_index,
        QPixmap(":/icon/Gnome-Bookmark-New-32.png"),
        bookmark_name);
}

void MainWindow::on_actionLoad_from_file_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open log file"), "",
        tr("TextFiles (*.txt);;All Files (*)"));
    if (filename.isEmpty())
        return;

    spawnViewerWithContent(Logfile(filename));
}

void MainWindow::on_actionGrep_current_view_triggered()
{
    grepCurrentView();
}
void MainWindow::on_actionBookmark_current_line_triggered()
{
    bookmarkCurrentLine();
}

void MainWindow::on_exit_app_triggered()
{
    QApplication::exit();
}
