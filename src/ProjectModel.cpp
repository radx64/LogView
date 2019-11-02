#include "ProjectModel.hpp"

ProjectModel::ProjectModel() : projectName_("<empty>")
{
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
    emit changed();
}

void ProjectModel::mocked_change()
{
    //TODO: This should be detectable by changing project model
    emit changed();
}
