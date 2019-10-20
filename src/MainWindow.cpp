#include "MainWindow.hpp"
#include "ui_MainWindow.h"

// TODO: cleanup this includes after some mockups creation and proper class segregation
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextEdit>

#include "Bookmark.hpp"
#include "BookmarksModel.hpp"
#include "GrepDialogWindow.hpp"
#include "GrepNode.hpp"
#include "Logfile.hpp"
#include "ProjectModel.hpp"
#include "TabCompositeViewer.hpp"
#include "TextRenderer.hpp"
#include "Viewer.hpp"
#include "loader/Project.hpp"
#include "serializer/SerializerProjectModel.hpp"

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
        spawnViewerWithContent(fileList.toLocalFile());
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

void MainWindow::spawnViewerWithContent(QString file_path)
{
//    QTabWidget* fileTabWidget = ui->fileView;
//    QString filename = log->getFileName();
//    Viewer* viewer = new Viewer(fileTabWidget, std::move(log));
//    fileTabWidget ->addTab(viewer, filename.split(QRegularExpression("[\\/]")).last());

    std::unique_ptr<ProjectModel> project = std::make_unique<ProjectModel>();
    project->logfile_model_ = std::make_unique<Logfile>(file_path);
    project->grep_hierarchy_ = std::make_unique<GrepNode>("ROOT");
    project->file_path_ = file_path;
    loader::Project::load(ui, std::move(project));

}

Viewer* MainWindow::get_active_viewer_widget()
{
    const int tab_index = ui->fileView->currentIndex();
    if(tab_index == -1) return nullptr;
    qDebug() << "File tab index:" <<tab_index;
    Viewer* viewerWidget = dynamic_cast<Viewer*>(ui->fileView->widget(tab_index));
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

    Viewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message

    TabCompositeViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    GrepDialogWindow grepDialog;

    if (grepDialog.exec() != QDialog::Accepted) return;
    auto result = grepDialog.getResult();
    if (deepest_tab) deepest_tab->grep(result.pattern, result.is_regex, result.is_case_insensitive);

    //For debug
    viewerWidget->project_model_->grep_hierarchy_->evaluate(0);
}

void MainWindow::bookmarkCurrentLine()
{
    Viewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message
    TabCompositeViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    if (!deepest_tab) return;
    int current_line_index = deepest_tab->text_ ->textCursor().blockNumber();
    uint32_t absolute_line_index = deepest_tab->lines_[current_line_index].number;

    // Simple QInputDialog will be extended later for something more fancy
    bool ok = false;
    QString bookmark_name = QInputDialog::getText(this, tr("Bookmark creation"),
        tr("Name:"), QLineEdit::Normal, deepest_tab->lines_[current_line_index].text, &ok);

    if (!ok) return;
    qDebug() << "Adding bookmark at line" << absolute_line_index;
    viewerWidget->project_model_->getBookmarksModel()->add_bookmark(absolute_line_index,
        QString(":/icon/Gnome-Bookmark-New-32.png"),
        bookmark_name);
}

void MainWindow::on_actionLoad_from_file_triggered()
{
    QString file_path = QFileDialog::getOpenFileName(this,
        tr("Open log file"), "",
        tr("TextFiles (*.txt);;All Files (*)"));
    if (file_path.isEmpty())
        return;

    spawnViewerWithContent(file_path);
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

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, QString("About application"), QString("RadX64 © 2019\nReleased under\nGNU GENERAL PUBLIC LICENSE"));
}

void MainWindow::on_actionSave_project_triggered()
{
    // DUMMY JSON SERIALIZER TESTS
    Viewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message

    QFile saveFile("save.json");
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }

    QJsonObject object;
    serializer::ProjectModel::serialize(*viewerWidget->project_model_, object);
    QJsonDocument document(object);
    qDebug() << document.toJson(QJsonDocument::Indented);
    saveFile.write(document.toJson(QJsonDocument::Indented));
}

void MainWindow::on_actionLoad_project_triggered()
{
    // DUMMY JSON DESERIALIZER TESTS
    QFile loadFile("save.json");
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }
    QJsonDocument document = QJsonDocument::fromJson(loadFile.readAll());
    QJsonObject object = document.object();

    std::unique_ptr<ProjectModel> project = std::make_unique<ProjectModel>();
    serializer::ProjectModel::deserialize(*project, object);
    loader::Project::load(ui, std::move(project));
}
