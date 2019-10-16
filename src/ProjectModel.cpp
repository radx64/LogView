#include "ProjectModel.hpp"

#include <memory>

#include "BookmarksModel.hpp"
#include "Logfile.hpp"
#include "GrepNode.hpp"

ProjectModel::ProjectModel()
{
    bookmarks_model_ = std::make_unique<BookmarksModel>(nullptr);
}

BookmarksModel* ProjectModel::getBookmarksModel()
{
    return bookmarks_model_.get();
}

void ProjectModel::serialize(QJsonObject &json) const
{
    json["filepath"] = file_path_;
    QJsonObject greps;
    grep_hierarchy_->serialize(greps);
    json["greps"] = greps;
    bookmarks_model_->serialize(json);
}
void ProjectModel::deserialize(const QJsonObject &json)
{
    (void)json;
}
