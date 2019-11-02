#include "SerializerLogfile.hpp"

#include <QJsonObject>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"
#include "../Logfile.hpp"

#include "SerializerGrepNode.hpp"
#include "SerializerBookmarksModel.hpp"

namespace serializer
{

void Logfile::serialize(const ::Logfile &lf, QJsonObject &json)
{
    json["filepath"] = lf.filename_;
    QJsonObject greps;
    GrepNode::serialize(*lf.grep_hierarchy_, greps);

    json["greps"] = greps;
    BookmarksModel::serialize(*lf.bookmarks_model_, json);
}
void Logfile::deserialize(::Logfile &lf, const QJsonObject &json)
{
    lf.filename_ = json["filepath"].toString();

    std::unique_ptr<::GrepNode> gn = std::make_unique<::GrepNode>();
    serializer::GrepNode::deserialize(*gn, json["greps"].toObject());
    lf.grep_hierarchy_ = std::move(gn);

    std::unique_ptr<::BookmarksModel> bm = std::make_unique<::BookmarksModel>();
    serializer::BookmarksModel::deserialize(*bm, json);
    lf.bookmarks_model_ = std::move(bm);

    lf.connect_events();
}

}  // namespace serializer
