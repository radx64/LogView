#pragma once

#include <functional>
#include <memory>

#include <QHash>
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
class ArchiveDocument;
class ArchiveViewer;
class BackgroundTask;
class QTabWidget;

class ProjectUiManager : public QObject
{
Q_OBJECT
public:
    ProjectUiManager(Ui::MainWindow* ui);
    ~ProjectUiManager() override;
    void create_new();
    void load_log_file(QString file_path);
    void beginLoadingLogfile(Logfile* lf, FileViewer* viewer, QWidget* progress_tab_host);
    void onArchiveViewerClosing(ArchiveDocument* archive);
    void reload_file(int tab_index);
    void reloadViewer(QTabWidget* tabs, int index, QWidget* progress_host, bool project_file);
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
    void archive_viewer_closed(ArchiveDocument* archive);

private:
    struct LoadEntry
    {
        BackgroundTask* loader = nullptr;
        QPointer<FileViewer> viewer;
        QPointer<QWidget> progress_tab;
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
    void present_archive(ArchiveDocument* archive);
    void load_archive_file(QString file_path);
    void beginArchiveCatalog(ArchiveDocument* archive, ArchiveViewer* viewer);
    void cancelArchiveCatalog(ArchiveDocument* archive);
    void beginLoading(Logfile* lf, FileViewer* viewer, QWidget* progress_tab_host);
    void onLoaderFinished(BackgroundTask* loader, bool ok);
    void setTabLoading(LoadEntry& entry, bool loading);
    void emitAggregateProgress();
    void regenerateAutoBookmarks();

    std::unique_ptr<ProjectModel> pm_;
    Ui::MainWindow* ui_;
    QList<LoadEntry> active_loads_;
    QHash<ArchiveDocument*, QPointer<BackgroundTask>> catalog_tasks_;
};

