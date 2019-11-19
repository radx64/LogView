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
    ProjectUiManager(Ui::MainWindow* ui);
    void create_new();
    void load_log_file(QString file_path);
    void connect_update_notif(std::function<void(void)> notif_callback);
    bool is_empty();
    bool has_changed();
    const QString& project_name();
    void save_empty_project();
    void save_project();
    void open_project();

private slots:
    void project_changed();

private:
    std::unique_ptr<ProjectModel> pm_;
    std::function<void(void)> update_client_notif_;
    Ui::MainWindow* ui_;
};


#endif // PROJECT_MANAGER_HPP
