#include "ProjectModel.hpp"
#include <QDebug>


ProjectModel::ProjectModel() : projectName_("<empty>"), changed_{false}
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
