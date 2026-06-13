#include "ProjectUiManager.hpp"

#include "FileViewer.hpp"

#include <QDebug>

ProjectUiManager::ProjectUiManager(Ui::MainWindow* ui)
: ui_(ui)
{
    pm_ = std::make_unique<ProjectModel>();
    adoptProjectModel();
}

void ProjectUiManager::create_new()
{
    clearViews();
    pm_ = std::make_unique<ProjectModel>();
    adoptProjectModel();
}

void ProjectUiManager::adoptProjectModel()
{
    connect(pm_.get(), &ProjectModel::stateChanged,
            this, &ProjectUiManager::projectStateChanged);
    emit projectStateChanged();
}

void ProjectUiManager::clearViews()
{
    while (ui_->fileView->count())
    {
        QWidget* tab = ui_->fileView->widget(0);
        ui_->fileView->removeTab(0);
        delete tab;
    }
}

void ProjectUiManager::load_log_file(QString file_path)
{
    Logfile* lf = pm_->add_to_project(std::make_unique<Logfile>(file_path));
    loader::Logfile::load(
        ui_,
        lf,
        [&](FileViewer* fileviewer)
        {
            connect_logviewer_signal(fileviewer);
        });
    pm_->markDirty();
}

void ProjectUiManager::reload_file(int tab_index)
{
    FileViewer* viewer = dynamic_cast<FileViewer*>(ui_->fileView->widget(tab_index));
    if (viewer == nullptr)
        return;

    Logfile* lf = viewer->logfile_;
    const QString tab_text = ui_->fileView->tabText(tab_index);
    const QString tab_tooltip = ui_->fileView->tabToolTip(tab_index);

    lf->reload();

    disconnect(viewer, &FileViewer::destroyed, this, &ProjectUiManager::file_viewer_closed);
    ui_->fileView->removeTab(tab_index);
    delete viewer;

    FileViewer* reloaded = new FileViewer(ui_->fileView, lf);
    connect_logviewer_signal(reloaded);
    ui_->fileView->insertTab(tab_index, reloaded, tab_text);
    ui_->fileView->setTabToolTip(tab_index, tab_tooltip);
    ui_->fileView->setCurrentIndex(tab_index);

    loader::Logfile::spawnViews(reloaded->getDeepestActiveTab(), lf->grep_hierarchy_.get());
}

void ProjectUiManager::on_logfile_wiget_close(Logfile* lf)
{
    qDebug() << "Clean some projet resources here";
    pm_->remove_file_from_project(lf);
}

bool ProjectUiManager::is_empty()
{
    return pm_->is_empty();
}
bool ProjectUiManager::has_changed()
{
    return pm_->isDirty();
}

const QString& ProjectUiManager::project_name()
{
    return pm_->name();
}

void ProjectUiManager::save_project()
{
    QString file_path = project_name();

    if (file_path.isEmpty())
    {
        file_path = QFileDialog::getSaveFileName(ui_->fileView->window(),
            tr("Save project"), "",
            tr("Project file (*.json)"));
        qDebug() << "FP: " << file_path;
    }

    if (file_path.isEmpty())
        return;

    QFile saveFile(file_path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }
    pm_->setName(saveFile.fileName());

    QJsonObject object;
    serializer::ProjectModel::serialize(*pm_, object);
    QJsonDocument document(object);
    saveFile.write(document.toJson(QJsonDocument::Indented));

    pm_->markClean();
}

void ProjectUiManager::open_project()
{
    QString file_path = QFileDialog::getOpenFileName(ui_->fileView->window(),
                                                     tr("Open project"), "",
                                                     tr("Project file (*.json)"));
    if (file_path.isEmpty())
        return;

    clearViews();

    pm_ = std::make_unique<ProjectModel>();
    adoptProjectModel();

    QFile loadFile(file_path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }
    QJsonDocument document = QJsonDocument::fromJson(loadFile.readAll());
    QJsonObject object = document.object();

    serializer::ProjectModel::deserialize(*pm_, object);
    loader::Project::load(
        ui_,
        pm_.get(),
        [&](FileViewer* fileviewer)
        {
            connect_logviewer_signal(fileviewer);
        });
    pm_->markClean();
}

void ProjectUiManager::connect_logviewer_signal(FileViewer* fileviewer)
{
    connect(fileviewer, &FileViewer::destroyed, this, &ProjectUiManager::file_viewer_closed);
}

void ProjectUiManager::file_viewer_closed(Logfile* lf)
{
    qDebug() << "Closed viewer: " << lf;
    pm_->remove_file_from_project(lf);
}
