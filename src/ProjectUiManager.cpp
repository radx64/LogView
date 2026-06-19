#include "ProjectUiManager.hpp"

#include "FileViewer.hpp"
#include "FileLineSource.hpp"
#include "Settings.hpp"
#include "loader/BackgroundTask.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

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
    present_logfile(lf);
    pm_->markDirty();
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
    beginLoading(lf, viewer);
}

void ProjectUiManager::beginLoading(Logfile* lf, FileViewer* viewer)
{
    std::shared_ptr<FileLineSource> source = lf->getFileSource();

    // Nothing to index (already loaded or invalid) -> show greps right away.
    if (!source || source->isLoaded())
    {
        loader::Logfile::spawnViews(viewer->getDeepestActiveTab(), lf->grep_hierarchy_.get());
        viewer->refreshAfterLoad();
        return;
    }

    LoadEntry entry;
    entry.loader = new BackgroundTask(
        [source](const BackgroundTask::ProgressFn& report) { return source->buildIndex(report); },
        this);
    entry.viewer = viewer;
    entry.lf = lf;
    entry.name = lf->getFileName().split(QRegularExpression("[\\/]")).last();
    entry.total = source->fileSize();
    setTabLoading(entry, true);
    active_loads_.append(entry);

    BackgroundTask* loader = entry.loader;

    // Stop indexing if the user closes the tab before loading finishes.
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
        entry.viewer->refreshAfterLoad();
    }

    loader->deleteLater();
    emitAggregateProgress();
}

void ProjectUiManager::setTabLoading(LoadEntry& entry, bool loading)
{
    if (!entry.viewer)
        return;

    const int idx = ui_->fileView->indexOf(entry.viewer);
    if (idx < 0)
        return;

    if (loading)
    {
        entry.tab_base = ui_->fileView->tabText(idx);
        ui_->fileView->setTabText(idx, entry.tab_base + tr(" (loading…)"));
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
        text = tr("Loading %1… %2%").arg(active_loads_.first().name).arg(percent);
    else
        text = tr("Loading %1 files… %2%").arg(active_loads_.size()).arg(percent);

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

    beginLoading(lf, reloaded);
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
            tr("Save project"),
            Settings::instance().lastOpenedProjectDirectory(),
            tr("Project file (*.json)"));
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
                                            tr("Open project"),
                                            Settings::instance().lastOpenedProjectDirectory(),
                                            tr("Project file (*.json)"));
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
