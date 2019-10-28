#ifndef PROJECT_MODEL_MANAGER_HPP
#define PROJECT_MODEL_MANAGER_HPP

#include <memory>
#include <vector>
#include <QString>

#include "ProjectModel.hpp"
#include "Logfile.hpp"
#include "GrepNode.hpp"
#include "BookmarksModel.hpp"

class ProjectModel;
class Logfile;

class ProjectModelManager
{
public:
ProjectModelManager() = default;

ProjectModel* create(std::unique_ptr<Logfile> log)
{
    auto pm = std::make_unique<ProjectModel>();

    pm = std::make_unique<ProjectModel>();
    pm->file_path_ = log->getFileName();
    pm->grep_hierarchy_ = std::make_unique<GrepNode>("ROOT");
    pm->logfile_model_ = std::move(log);

    storage_.push_back(std::move(pm));
    return storage_.back().get();
}

ProjectModel* add(std::unique_ptr<ProjectModel>&& pm)
{
    storage_.push_back(std::move(pm));
    return storage_.back().get();
}

//TODO: implement project removing from manager on tab close;

protected:
    std::vector<std::unique_ptr<ProjectModel>> storage_;
};

#endif // PROJECT_MODEL_MANAGER_HPP
