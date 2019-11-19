#ifndef PROJECT_MANAGER_HPP
#define PROJECT_MANAGER_HPP

#include <functional>
#include <memory>

#include <QObject>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

#include "ProjectModel.hpp"
#include "loader/LoaderLogFile.hpp"
#include "loader/Project.hpp"
#include "ui_MainWindow.h"
#include "Logfile.hpp"
#include "serializer/SerializerProjectModel.hpp"

class ProjectUiManager : public QObject
{
Q_OBJECT
public:

    ProjectUiManager(Ui::MainWindow* ui) : ui_(ui)
    {
        pm_ = std::make_unique<ProjectModel>();
        pm_->changed_ = false;
    }

    void create_new()
    {
        pm_ = std::make_unique<ProjectModel>();
        pm_->changed_ = false;
    }

    void load_log_file(QString file_path)
    {
        Logfile* lf = pm_->add_to_project(std::make_unique<Logfile>(file_path));
        loader::Logfile::load(ui_, lf);
    }

    void connect_update_notif(std::function<void(void)> notif_callback)
    {
        update_client_notif_ = notif_callback;
    }

    bool is_empty()
    {
        return pm_->is_empty();
    }
    bool has_changed()
    {
        return pm_->changed_;
    }

    const QString& project_name()
    {
        return pm_->projectName_;
    }

    void save_empty_project()
    {
        //TODO below nullptr might be bad
        QString file_path = QFileDialog::getSaveFileName(nullptr,
            tr("Save project"), "",
            tr("Project file (*.json)"));

        if (file_path.isEmpty())
            return;

        QFile saveFile(file_path);
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
    }

    void save_project()
    {
        if (project_name().isEmpty())
        {
            save_empty_project();
            return;
        }

        //todo merge below code with save_empty_project
        QFile saveFile(project_name());
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning("Couldn't open save file!");
            return;
        }

        QJsonObject object;
        serializer::ProjectModel::serialize(*pm_, object);
        QJsonDocument document(object);
        saveFile.write(document.toJson(QJsonDocument::Indented));

        pm_->changed_ = false;
        update_client_notif_();
    }

    void open_project()
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
        loader::Project::load(ui_, pm_.get());
        pm_->changed_ = false;
    }


private slots:
    void project_changed()
    {
        update_client_notif_();
    }

private:
    std::unique_ptr<ProjectModel> pm_;
    std::function<void(void)> update_client_notif_;
    Ui::MainWindow* ui_;
};


#endif // PROJECT_MANAGER_HPP
