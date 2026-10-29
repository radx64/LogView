#include "ProjectUiManager.hpp"

#include "ArchiveDocument.hpp"
#include "ArchiveViewer.hpp"
#include "AutoBookmarksModel.hpp"
#include "FileViewer.hpp"
#include "FileLineSource.hpp"
#include "Settings.hpp"
#include "Translator.hpp"
#include "archive/ArchiveCatalog.hpp"
#include "archive/ArchiveCommon.hpp"
#include "loader/BackgroundTask.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

ProjectUiManager::ProjectUiManager(Ui::MainWindow* ui)
: ui_(ui)
{
    pm_ = std::make_unique<ProjectModel>();
    adoptProjectModel();

    connect(&AutoBookmarksModel::instance(), &AutoBookmarksModel::changed,
            this, &ProjectUiManager::regenerateAutoBookmarks);
}

ProjectUiManager::~ProjectUiManager()
{
    const QList<ArchiveDocument*> archives = catalog_tasks_.keys();
    for (ArchiveDocument* archive : archives)
        cancelArchiveCatalog(archive);
}

void ProjectUiManager::cancelArchiveCatalog(ArchiveDocument* archive)
{
    if (!archive)
        return;

    QPointer<BackgroundTask> task = catalog_tasks_.take(archive);
    if (!task)
        return;

    disconnect(task, nullptr, this, nullptr);
    task->cancel();
    task->deleteLater();
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
    if (logview_archive::isArchivePath(file_path))
    {
        load_archive_file(std::move(file_path));
        return;
    }

    Logfile* lf = pm_->add_to_project(std::make_unique<Logfile>(file_path));
    present_logfile(lf);
    pm_->markDirty();
}

void ProjectUiManager::load_archive_file(QString file_path)
{
    ArchiveDocument* archive =
        pm_->add_archive_to_project(std::make_unique<ArchiveDocument>(file_path));
    present_archive(archive);
    pm_->markDirty();
}

void ProjectUiManager::present_archive(ArchiveDocument* archive)
{
    ArchiveViewer* viewer = loader::Logfile::createArchiveView(
        ui_,
        archive,
        this,
        [](ArchiveViewer*) {});
    beginArchiveCatalog(archive, viewer);
}

void ProjectUiManager::beginArchiveCatalog(ArchiveDocument* archive, ArchiveViewer* viewer)
{
    cancelArchiveCatalog(archive);

    const QString archive_path = archive->archivePath();
    auto result = std::make_shared<ArchiveCatalogResult>();
    QPointer<ArchiveDocument> archive_guard(archive);
    QPointer<ArchiveViewer> viewer_guard(viewer);

    BackgroundTask* task = new BackgroundTask(
        [archive_path, result](const BackgroundTask::ProgressFn& report) -> bool
        {
            *result = ArchiveCatalog::build(archive_path,
                                            [&report](int count)
                                            {
                                                return report(count, qMax<qint64>(count, 1));
                                            });
            return true;
        },
        this);

    catalog_tasks_.insert(archive, task);

    connect(viewer, &QObject::destroyed, task, [task]() {
        if (task)
            task->cancel();
    });

    connect(task, &BackgroundTask::finished, this,
            [this, archive_guard, viewer_guard, result](bool ok)
            {
                if (archive_guard)
                    catalog_tasks_.remove(archive_guard.data());
                if (!archive_guard || !viewer_guard || !ok)
                    return;
                archive_guard->setCatalog(result->entry_paths, result->truncated);
            });
    connect(task, &BackgroundTask::finished, task, &BackgroundTask::deleteLater);
    task->start();
}

void ProjectUiManager::beginLoadingLogfile(Logfile* lf, FileViewer* viewer, QWidget* progress_tab_host)
{
    beginLoading(lf, viewer, progress_tab_host);
}

void ProjectUiManager::present_logfile(Logfile* lf)
{
    FileViewer* viewer = loader::Logfile::createView(
        ui_,
        lf,
        [&](FileViewer* fileviewer)
        {
            connect_logviewer_signal(fileviewer);
        });
    beginLoading(lf, viewer, viewer);
}

void ProjectUiManager::beginLoading(Logfile* lf, FileViewer* viewer, QWidget* progress_tab_host)
{
    std::shared_ptr<FileLineSource> source = lf->getFileSource();

    if (!source || source->isLoaded())
    {
        loader::Logfile::spawnViews(viewer->getDeepestActiveTab(), lf->grep_hierarchy_.get());
        lf->generateAutoBookmarks();
        viewer->refreshAfterLoad();
        return;
    }

    LoadEntry entry;
    entry.loader = new BackgroundTask(
        [source](const BackgroundTask::ProgressFn& report) { return source->buildIndex(report); },
        this);
    entry.viewer = viewer;
    entry.progress_tab = progress_tab_host ? progress_tab_host : viewer;
    entry.lf = lf;
    entry.name = lf->getFileName().split(QRegularExpression("[\\/]")).last();
    entry.total = source->fileSize();
    setTabLoading(entry, true);
    active_loads_.append(entry);

    BackgroundTask* loader = entry.loader;

    connect(viewer, &FileViewer::destroyed, loader, [loader]() { loader->cancel(); });

    connect(loader, &BackgroundTask::progress, this,
        [this, loader](qint64 done, qint64 total)
        {
            for (LoadEntry& e : active_loads_)
            {
                if (e.loader == loader)
                {
                    e.done = done;
                    e.total = total;
                    break;
                }
            }
            emitAggregateProgress();
        });

    connect(loader, &BackgroundTask::finished, this,
        [this, loader](bool ok) { onLoaderFinished(loader, ok); });

    emitAggregateProgress();
    loader->start();
}

void ProjectUiManager::onLoaderFinished(BackgroundTask* loader, bool ok)
{
    int index = -1;
    for (int i = 0; i < active_loads_.size(); ++i)
    {
        if (active_loads_[i].loader == loader)
        {
            index = i;
            break;
        }
    }
    if (index < 0)
        return;

    LoadEntry entry = active_loads_.takeAt(index);

    if (ok && entry.viewer)
    {
        setTabLoading(entry, false);
        loader::Logfile::spawnViews(entry.viewer->getDeepestActiveTab(),
                                    entry.lf->grep_hierarchy_.get());
        entry.lf->generateAutoBookmarks();
        entry.viewer->refreshAfterLoad();
    }

    loader->deleteLater();
    emitAggregateProgress();
}

void ProjectUiManager::setTabLoading(LoadEntry& entry, bool loading)
{
    QWidget* tab_widget = entry.progress_tab ? entry.progress_tab.data() : entry.viewer.data();
    if (!tab_widget)
        return;

    const int idx = ui_->fileView->indexOf(tab_widget);
    if (idx < 0)
        return;

    if (loading)
    {
        entry.tab_base = ui_->fileView->tabText(idx);
        ui_->fileView->setTabText(idx, entry.tab_base + Lang::tr("tab.loading_suffix"));
    }
    else
    {
        ui_->fileView->setTabText(idx, entry.tab_base);
    }
}

void ProjectUiManager::emitAggregateProgress()
{
    if (active_loads_.isEmpty())
    {
        emit loadProgressChanged(false, 0, QString());
        return;
    }

    qint64 done = 0;
    qint64 total = 0;
    for (const LoadEntry& e : active_loads_)
    {
        done += e.done;
        total += e.total;
    }

    const int percent = (total > 0)
        ? static_cast<int>(done * 100 / total)
        : 0;

    QString text;
    if (active_loads_.size() == 1)
        text = Lang::tr("status.loading_one").arg(active_loads_.first().name).arg(percent);
    else
        text = Lang::tr("status.loading_many").arg(active_loads_.size()).arg(percent);

    emit loadProgressChanged(true, percent, text);
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

    beginLoading(lf, reloaded, reloaded);
}

void ProjectUiManager::on_logfile_wiget_close(Logfile* lf)
{
    qDebug() << "Clean some projet resources here";
    pm_->remove_file_from_project(lf);
}

void ProjectUiManager::regenerateAutoBookmarks()
{
    for (const std::unique_ptr<Logfile>& lf : pm_->get_log_files())
        if (lf)
            lf->generateAutoBookmarks();
    for (const std::unique_ptr<ArchiveDocument>& archive : pm_->get_archives())
    {
        if (!archive)
            continue;
        for (const ArchiveDocument::OpenedEntry& entry : archive->openedEntries())
        {
            if (entry.logfile)
                entry.logfile->generateAutoBookmarks();
        }
    }
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
            Lang::tr("dialog.save_project.title"),
            Settings::instance().lastOpenedProjectDirectory(),
            Lang::tr("filter.project_file"));
        qDebug() << "FP: " << file_path;
    }

    if (file_path.isEmpty())
        return;

    Settings::instance().setLastOpenedProjectDirectory(
        QFileInfo(file_path).absolutePath());

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

QString ProjectUiManager::open_project(const QString& file_path)
{
    QString path = file_path;
    if (path.isEmpty())
    {
        path = QFileDialog::getOpenFileName(ui_->fileView->window(),
                                            Lang::tr("dialog.open_project.title"),
                                            Settings::instance().lastOpenedProjectDirectory(),
                                            Lang::tr("filter.project_file"));
    }
    if (path.isEmpty())
        return QString();

    Settings::instance().setLastOpenedProjectDirectory(
        QFileInfo(path).absolutePath());

    QFile loadFile(path);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file!");
        return QString();
    }

    clearViews();

    pm_ = std::make_unique<ProjectModel>();
    adoptProjectModel();

    QJsonDocument document = QJsonDocument::fromJson(loadFile.readAll());
    QJsonObject object = document.object();

    serializer::ProjectModel::deserialize(*pm_, object);
    for (const auto& logfile : pm_->get_log_files())
        present_logfile(logfile.get());
    for (const auto& archive : pm_->get_archives())
        present_archive(archive.get());
    pm_->markClean();
    return path;
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

void ProjectUiManager::onArchiveViewerClosing(ArchiveDocument* archive)
{
    archive_viewer_closed(archive);
}

void ProjectUiManager::archive_viewer_closed(ArchiveDocument* archive)
{
    if (!archive)
        return;

    cancelArchiveCatalog(archive);
    archive->blockSignals(true);
    pm_->remove_archive_from_project(archive);
}
