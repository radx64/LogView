#include "ProjectUiManager.hpp"
#include <QDebug>

ProjectUiManager::ProjectUiManager(Ui::MainWindow* ui) : ui_(ui)
{
    pm_ = std::make_unique<ProjectModel>();
    pm_->changed_ = false;
}

void ProjectUiManager::create_new()
{
    pm_ = std::make_unique<ProjectModel>();
    pm_->changed_ = false;
}

void ProjectUiManager::load_log_file(QString file_path)
{
    Logfile* lf = pm_->add_to_project(std::make_unique<Logfile>(file_path));
    loader::Logfile::load(ui_, lf, [&](){this->on_logfile_wiget_close(lf);});
}

void ProjectUiManager::on_logfile_wiget_close(Logfile* lf)
{
    qDebug() << "Clean some projet resources here";
    pm_->remove_file_from_project(lf);
}

void ProjectUiManager::connect_update_notif(std::function<void(void)> notif_callback)
{
    update_client_notif_ = notif_callback;
}

bool ProjectUiManager::is_empty()
{
    return pm_->is_empty();
}
bool ProjectUiManager::has_changed()
{
    return pm_->changed_;
}

const QString& ProjectUiManager::project_name()
{
    return pm_->projectName_;
}

void ProjectUiManager::save_project()
{
    QString file_path = project_name();

    if (file_path.isEmpty())
    {
        file_path = QFileDialog::getSaveFileName(nullptr,
            tr("Save project"), "",
            tr("Project file (*.json)"));
    }

    if (file_path.isEmpty())
        return;

    QFile saveFile(project_name());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }
    pm_->projectName_ = saveFile.fileName();

    QJsonObject object;
    serializer::ProjectModel::serialize(*pm_, object);
    QJsonDocument document(object);
    saveFile.write(document.toJson(QJsonDocument::Indented));

    pm_->changed_ = false;
    update_client_notif_();
}

void ProjectUiManager::open_project()
{
    while(ui_->fileView->count())
    {
        ui_->fileView->removeTab(0);
    }
    ui_->fileView->clear();

    pm_ = std::make_unique<ProjectModel>();

    //Todo fix below nullptr
    QString file_path = QFileDialog::getOpenFileName(nullptr,
                                                     tr("Open project"), "",
                                                     tr("Project file (*.json)"));
    if (file_path.isEmpty())
        return;

    QFile loadFile(file_path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file!");
        return;
    }
    QJsonDocument document = QJsonDocument::fromJson(loadFile.readAll());
    QJsonObject object = document.object();

    serializer::ProjectModel::deserialize(*pm_, object);
    QObject::connect(pm_.get(), &ProjectModel::changed, this, &ProjectUiManager::project_changed);
    loader::Project::load(ui_, pm_.get(), [&](Logfile* lf){this->on_logfile_wiget_close(lf);});
    pm_->changed_ = false;
}

void ProjectUiManager::project_changed()
{
    update_client_notif_();
}
