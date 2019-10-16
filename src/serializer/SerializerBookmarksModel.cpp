#include "SerializerBookmarksModel.hpp"

#include "../BookmarksModel.hpp"
#include "SerializerBookmark.hpp"

#include <QJsonArray>
#include <QJsonObject>

namespace serializer
{
void BookmarksModel::serialize(const ::BookmarksModel& bmodel, QJsonObject &json)
{
    QJsonArray array;
    for (const auto& bookmark : bmodel.bookmarks_)
    {
        QJsonObject jsonBookmark;
        Bookmark::serialize(bookmark, jsonBookmark);
        array.append(jsonBookmark);
    }
    json["bookmarks"] = array;
}

void BookmarksModel::deserialize(::BookmarksModel& bmodel, const QJsonObject &json)
{
    (void) bmodel;
    (void) json;
}

}  // namespace serializer

