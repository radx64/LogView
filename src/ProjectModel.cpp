#include "ProjectModel.hpp"
#include <QDebug>


ProjectModel::ProjectModel() : projectName_{""}, changed_{false}
{
}

ProjectModel::~ProjectModel()
{

}

Logfile* ProjectModel::add_to_project(std::unique_ptr<Logfile>&& lf)
{
    logfiles_.push_back(std::move(lf));
    auto moved_logfile = logfiles_.back().get();

    QObject::connect(moved_logfile, &Logfile::changed,
                     this, &ProjectModel::on_logfile_change);

    on_logfile_change();
    return moved_logfile;
}

std::vector<std::unique_ptr<Logfile>>& ProjectModel::get_log_files()
{
    return logfiles_;
}

void ProjectModel::on_logfile_change()
{
    changed_ = true;
    emit changed();
}

bool ProjectModel::is_empty()
{
    return logfiles_.empty();
}

void ProjectModel::remove_file_from_project(Logfile* logfile)
{
    auto logfile_it = std::find_if(logfiles_.begin(), logfiles_.end(),
        [logfile](auto& managed_logfile){return managed_logfile.get() == logfile;});

    if (logfile_it != logfiles_.end()) logfiles_.erase(logfile_it);
}
