#pragma once

#include <functional>
#include <memory>

#include <QList>
#include <QObject>
#include <QPointer>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

#include "ProjectModel.hpp"
#include "loader/LoaderLogFile.hpp"
#include "UiMainWindow.hpp"
#include "Logfile.hpp"
#include "serializer/SerializerProjectModel.hpp"

class FileViewer;
class FileLoader;

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
    // Aggregated background-load state. active=false means no file is loading.
    void loadProgressChanged(bool active, int percent, const QString& text);

private slots:
    void file_viewer_closed(Logfile* lf);

private:
    struct LoadEntry
    {
        FileLoader* loader = nullptr;
        QPointer<FileViewer> viewer;
        Logfile* lf = nullptr;
        QString name;
        QString tab_base;
        qint64 done = 0;
        qint64 total = 0;
    };

    void on_logfile_wiget_close(Logfile* lf);
    void connect_logviewer_signal(FileViewer* fileviewer);
    void adoptProjectModel();
    void clearViews();

    void present_logfile(Logfile* lf);
    void beginLoading(Logfile* lf, FileViewer* viewer);
    void onLoaderFinished(FileLoader* loader, bool ok);
    void setTabLoading(LoadEntry& entry, bool loading);
    void emitAggregateProgress();

    std::unique_ptr<ProjectModel> pm_;
    Ui::MainWindow* ui_;
    QList<LoadEntry> active_loads_;
};

