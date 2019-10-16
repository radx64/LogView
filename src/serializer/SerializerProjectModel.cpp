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
    (void) pm;
    (void) json;
}

}  // namespace serializer
