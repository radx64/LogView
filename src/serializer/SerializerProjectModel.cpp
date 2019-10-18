#include "SerializerProjectModel.hpp"

#include <QJsonObject>

#include "../ProjectModel.hpp"
#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"

#include "SerializerBookmarksModel.hpp"
#include "SerializerGrepNode.hpp"

namespace serializer
{

void ProjectModel::serialize(const ::ProjectModel &pm, QJsonObject &json)
{
    json["filepath"] = pm.file_path_;
    QJsonObject greps;
    GrepNode::serialize(*pm.grep_hierarchy_, greps);

    json["greps"] = greps;
    BookmarksModel::serialize(*pm.bookmarks_model_, json);
}
void ProjectModel::deserialize(::ProjectModel &pm, const QJsonObject &json)
{
    pm.file_path_ = json["filepath"].toString();

    std::unique_ptr<::GrepNode> gn = std::make_unique<::GrepNode>();
    serializer::GrepNode::deserialize(*gn, json["greps"].toObject());
    pm.grep_hierarchy_ = std::move(gn);

    std::unique_ptr<::BookmarksModel> bm = std::make_unique<::BookmarksModel>();
    serializer::BookmarksModel::deserialize(*bm, json);
    pm.bookmarks_model_ = std::move(bm);
}

}  // namespace serializer
