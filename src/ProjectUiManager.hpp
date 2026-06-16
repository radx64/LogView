#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

#include "ProjectModel.hpp"
#include "loader/LoaderLogFile.hpp"
#include "loader/Project.hpp"
#include "UiMainWindow.hpp"
#include "Logfile.hpp"
#include "serializer/SerializerProjectModel.hpp"

class FileViewer;

class ProjectUiManager : public QObject
{
Q_OBJECT
public:
    ProjectUiManager(Ui::MainWindow* ui);
    void create_new();
    void load_log_file(QString file_path);
    void reload_file(int tab_index);
    bool is_empty();
    bool has_changed();
    const QString& project_name();
    void save_project();
    QString open_project(const QString& file_path = QString());

signals:
    void projectStateChanged();

private slots:
    void file_viewer_closed(Logfile* lf);

private:
    void on_logfile_wiget_close(Logfile* lf);
    void connect_logviewer_signal(FileViewer* fileviewer);
    void adoptProjectModel();
    void clearViews();
    std::unique_ptr<ProjectModel> pm_;
    Ui::MainWindow* ui_;
};

