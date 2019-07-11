#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QLabel"
#include "QLayout"
#include "QTabWidget"
#include "QFileDialog"
#include "QMessageBox"
#include "QTextEdit"
#include "QAction"
#include "QMimeData"
#include "QInputDialog"
#include "QDebug"

#include "Viewer.hpp"
#include "ViewerWidget.hpp"
#include "TabCompositeViewer.hpp"

void MainWindow::closeFileTab(const int index)
{
    QTabWidget* tabWidget = ui->fileView;
    QWidget* tabContents = tabWidget->widget(index);
    tabWidget->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setAcceptDrops(true);
    ui->setupUi(this);

    ui->fileView->setTabsClosable(true);
    connect(ui->fileView, SIGNAL(tabCloseRequested(int)), this, SLOT(closeFileTab(int)));

    QAction *grep = new QAction(this);
    grep->setShortcut(Qt::Key_G | Qt::CTRL);

    connect(grep, SIGNAL(triggered()), this, SLOT(grepCurrentView()));
    this->addAction(grep);

    QAction *bookmark = new QAction(this);
    bookmark->setShortcut(Qt::Key_B | Qt::CTRL);

    connect(bookmark, SIGNAL(triggered()), this, SLOT(bookmarkCurrentLine()));
    this->addAction(bookmark);
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

    // extract the local paths of the files
    for (int i = 0; i < urlList.size(); ++i)
    {
      QString filename = urlList.at(i).toLocalFile();
      //qDebug( )<< ("Loading: " + filename.toStdString());
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly)) {
          QMessageBox::information(this, tr("Unable to open file"), file.errorString());
          return;
      }

      QStringList lines;
      while(!file.atEnd())
      {
          lines.append(file.readLine());
      }
      spawnViewerWithContent(filename.split("/").last(), lines);
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

void MainWindow::spawnViewerWithContent(QString& name, QStringList& content)
{
    QTabWidget* fileTabWidget = ui->fileView;
    ViewerWidget* viewer = new ViewerWidget(fileTabWidget);
    fileTabWidget ->addTab(viewer, name);

    //TODO: Change lines_ will be changed to some log model
    //  then this will be done on single invocation
    //  setText renders it via renderer but there is no acceess for that
    //  so for grep purposes it is held also as QStringList
    //  What a shame ;(
    viewer->logViewer_->text_->setText(content.join(""));
    viewer->logViewer_->lines_ = content;
}

TabCompositeViewer* travel_down_via_tabs(TabCompositeViewer* start_point)
{
    if (start_point == nullptr) return start_point;
    const int tab_grep_index = start_point->tabs_->currentIndex();
    qDebug() << "tab_grep_index:" << tab_grep_index;
    QWidget* active_tab = start_point->tabs_->widget(tab_grep_index);
    TabCompositeViewer* active_tab_casted = dynamic_cast<TabCompositeViewer*>(active_tab);
    if (active_tab_casted == nullptr) return start_point;
    TabCompositeViewer* result = travel_down_via_tabs(active_tab_casted);
    return result ? result : start_point;
}

void MainWindow::grepCurrentView()
{
    /*
     * This is totally experimental approach with recursive components search
     * It works but I hate it due those dynamic casts
     * Need to change approach to create model of greps and then render it recursively
     */

    const int tab_index = ui->fileView->currentIndex();
    qDebug() << "File tab index:" <<tab_index;

    ViewerWidget* viewerWidget = dynamic_cast<ViewerWidget*>(ui->fileView->widget(tab_index));
    qDebug() << viewerWidget;
    if (viewerWidget == nullptr) return;

    QWidget* deepest_tab = travel_down_via_tabs(viewerWidget->logViewer_);
    TabCompositeViewer* deepest_tab_casted = dynamic_cast<TabCompositeViewer*>(deepest_tab);

    bool ok = false;
    QString input_grep = QInputDialog::getText(this, tr("Enter grep pattern.."),
        tr("Grep:"), QLineEdit::Normal, "", &ok);

    if (deepest_tab_casted != nullptr && ok) deepest_tab_casted->grep(input_grep);

}

void MainWindow::bookmarkCurrentLine()
{
    qDebug() << "Would normally bookmark current line";
}

void MainWindow::on_actionLoad_from_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open log file"), "",
        tr("TextFiles (*.txt);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
    }

    QStringList lines;
    while(!file.atEnd())
    {
        lines.append(file.readLine());
    }
    spawnViewerWithContent(fileName.split("/").last(), lines);
}
