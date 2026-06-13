#include "ProjectModel.hpp"
#include <QDebug>


ProjectModel::ProjectModel() : projectName_{""}, dirty_{false}
{
}

ProjectModel::~ProjectModel()
{
    qDebug() << "Destroying ProjectModel @" << this;
}

Logfile* ProjectModel::add_to_project(std::unique_ptr<Logfile>&& lf)
{
    logfiles_.push_back(std::move(lf));
    auto moved_logfile = logfiles_.back().get();

    QObject::connect(moved_logfile, &Logfile::changed,
                     this, &ProjectModel::on_logfile_change);

    return moved_logfile;
}

std::vector<std::unique_ptr<Logfile>>& ProjectModel::get_log_files()
{
    return logfiles_;
}

void ProjectModel::on_logfile_change()
{
    markDirty();
}

const QString& ProjectModel::name() const
{
    return projectName_;
}

void ProjectModel::setName(const QString& name)
{
    projectName_ = name;
    emit stateChanged();
}

bool ProjectModel::isDirty() const
{
    return dirty_;
}

void ProjectModel::markDirty()
{
    dirty_ = true;
    emit stateChanged();
}

void ProjectModel::markClean()
{
    dirty_ = false;
    emit stateChanged();
}

bool ProjectModel::is_empty()
{
    return logfiles_.empty();
}

void ProjectModel::remove_file_from_project(Logfile* logfile)
{
    auto logfile_it = std::find_if(logfiles_.begin(), logfiles_.end(),
        [logfile](auto& managed_logfile){return managed_logfile.get() == logfile;});

    if (logfile_it != logfiles_.end())
    {
        qDebug() << "Erasing logfile from memory @" << logfile_it->get();
        logfiles_.erase(logfile_it);
        markDirty();
    }
}
