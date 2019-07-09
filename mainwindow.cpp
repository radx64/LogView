#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include "QLabel"
#include "QLayout"
#include "QTabWidget"
#include "QFileDialog"
#include "QMessageBox"
#include "QTextEdit"
#include "QAction"
#include "QMimeData"

Viewer::Viewer(QWidget* parent) : parent_(parent)
{
    text_ = new QTextEdit();
    text_->setReadOnly(true);
    text_->setText("Viewer");
    this->setLayout(new QHBoxLayout());
    text_->setParent(this);
}

TabCompositeViewer::TabCompositeViewer(QWidget* parent) : Viewer(parent)
{
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setParent(this);
    tabs_->setTabsClosable(true);
    connect(tabs_, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void TabCompositeViewer::grep()
{
    tabs_ ->addTab(new TabCompositeViewer(this),"ClonedDown");
}

void TabCompositeViewer::closeTab(const int index)
{
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setAcceptDrops(true);

    QTabWidget* tabWiget = new QTabWidget();
    ui->setupUi(this);
    setCentralWidget(tabWiget);

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
    std::cout << "Something was dropped here" << std::endl;
    event->acceptProposedAction();

    const QMimeData* mimeData = event->mimeData();

    if (!mimeData->hasUrls())
    {
        std::cout << "Non URL mime data type" << std::endl;
        return;
    }

    QList<QUrl> urlList = mimeData->urls();

    // extract the local paths of the files
    for (int i = 0; i < urlList.size(); ++i)
    {
      QString filename = urlList.at(i).toLocalFile();
      std::cout << "Loading: " + filename.toStdString() << std::endl;
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly)) {
          QMessageBox::information(this, tr("Unable to open file"), file.errorString());
          return;
      }

      QString textString;
      textString = file.readAll();
      spawnEditorWithContent(filename.split("/").last(), textString);
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

void MainWindow::spawnEditorWithContent(QString& name, QString& content)
{
    QTabWidget* tabWidget = static_cast<QTabWidget*>(this->centralWidget());
    QTabWidget* innerTabWidget = new QTabWidget();
    QTextEdit* myTextEditor = new QTextEdit();
    myTextEditor->setFontFamily("Courier New");
    myTextEditor->setText(content);
    myTextEditor->setReadOnly(true);

    innerTabWidget->addTab(myTextEditor, "Base");
    tabWidget->addTab(innerTabWidget, name);
    spawnedTabs_.push_back(innerTabWidget);

    TabCompositeViewer* v = new TabCompositeViewer(this);

    tabWidget->addTab(v, "ViewerTest");
    v->grep();
}

void MainWindow::grepCurrentView()
{
    std::cout << "Would normally grep active card"<< std::endl;
}

void MainWindow::bookmarkCurrentLine()
{
    std::cout << "Would normally bookmark current line"<< std::endl;
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

    QString textString;
    textString = file.readAll();
    spawnEditorWithContent(fileName.split("/").last(), textString);
}
