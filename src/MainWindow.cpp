#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <memory>

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
#include "LogViewer.hpp"
#include "TextRenderer.hpp"
#include "ProjectViewer.hpp"
#include "loader/Project.hpp"
#include "serializer/SerializerProjectModel.hpp"
#include "Version.hpp"
#include "ProjectUiManager.hpp"

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

void MainWindow::newProject()
{
    pm_->create_new();
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
    pm_ = std::make_unique<ProjectUiManager>(ui);
    pm_->connect_update_notif([this](){updateUi();});
    newProject();
    updateUi();
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
        load_log_file(fileList.toLocalFile());
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

void MainWindow::load_log_file(QString file_path)
{
   pm_->load_log_file(file_path);
}

ProjectViewer* MainWindow::get_active_viewer_widget()
{
    const int tab_index = ui->fileView->currentIndex();
    if(tab_index == -1) return nullptr;
    ProjectViewer* viewerWidget = dynamic_cast<ProjectViewer*>(ui->fileView->widget(tab_index));
    if (!viewerWidget) throw std::string("Could not find active ViewerWidget");
    return viewerWidget;
}

void MainWindow::grepCurrentView()
{
    ProjectViewer* viewerWidget = get_active_viewer_widget();
    assert(viewerWidget != nullptr);

    if (!viewerWidget) return; // can display here some message

    LogViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    GrepDialogWindow grepDialog;

    if (grepDialog.exec() != QDialog::Accepted) return;
    auto result = grepDialog.getResult();
    if (deepest_tab)
    {
        GrepNode* new_grep_node = new GrepNode(result.pattern.toStdString(),
                                               result.is_regex,
                                               result.is_case_insensitive,
                                               result.is_inverted);

        deepest_tab->grep(new_grep_node);
        deepest_tab->getGrepNode()->addChild(new_grep_node);
    }
}

void MainWindow::bookmarkCurrentLine()
{
    ProjectViewer* viewerWidget = get_active_viewer_widget();
    if (!viewerWidget) return; // can display here some message
    LogViewer* deepest_tab = viewerWidget->getDeepestActiveTab();

    if (!deepest_tab) return;
    int current_line_index = deepest_tab->text_ ->textCursor().blockNumber();
    uint32_t absolute_line_index = deepest_tab->lines_[current_line_index].number;

    // Simple QInputDialog will be extended later for something more fancy
    bool ok = false;
    QString bookmark_name = QInputDialog::getText(this, tr("Bookmark creation"),
        tr("Name:"), QLineEdit::Normal, deepest_tab->lines_[current_line_index].text, &ok);

    if (!ok) return;
    viewerWidget->logfile_->getBookmarksModel()->add_bookmark(absolute_line_index,
        QString(":/icon/Gnome-Bookmark-New-32.png"),
        bookmark_name);
}

void MainWindow::on_actionLoad_from_file_triggered()
{
    QString file_path = QFileDialog::getOpenFileName(this,
        tr("Open log file"), "",
        tr("All Files (*)"));
    if (file_path.isEmpty())
        return;

    load_log_file(file_path);
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
    if (pm_->has_changed())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save changes you made in current project? All changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveProject();
    }   //duplicate code (move it to separate function later)

    QApplication::exit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, QString("About application"),
        QString("LogView: " + QString(APP_VERSION) +
            "\nBuild: " + __DATE__ + " " + __TIME__ +
            "\n\nRadX64 © 2019\nReleased under\nGNU GENERAL PUBLIC LICENSE"));
}

void MainWindow::on_actionSave_project_as_triggered()
{
    if (pm_->is_empty())
    {
      QMessageBox::warning(this,"Warning!","Nothing to save.\nLoad file or project first.");
      return;
    }

    saveProject();
    updateUi();
}

void MainWindow::on_actionLoad_project_triggered()
{
    if (pm_->has_changed())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save changes you made in current project? All changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveProject();
    }

    openProject();
    updateUi();
}

void MainWindow::setWindowTitle(const QString& title)
{
    QMainWindow::setWindowTitle("LogView " + QString(APP_VERSION) +"\t" + title);
}

void MainWindow::refreshWindowTitle()
{
    setWindowTitle(pm_->project_name().isEmpty()?"":"  -  " +
                   pm_->project_name() +
                   QString(pm_->has_changed()?" *":""));
}

void MainWindow::project_changed()
{
    updateUi();
}

void MainWindow::on_actionSave_project_triggered()
{
    pm_->save_project();
}

void MainWindow::updateMenus()
{
    ui->actionSave_project->setEnabled(pm_->has_changed());
}

void MainWindow::updateUi()
{
    refreshWindowTitle();
    updateMenus();
}

void MainWindow::saveProject()
{
    pm_->save_empty_project();
}
void MainWindow::openProject()
{
    pm_->open_project();

}
