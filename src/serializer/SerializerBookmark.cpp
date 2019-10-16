#include "SerializerBookmark.hpp"

#include "../Bookmark.hpp"

#include <QJsonObject>

namespace serializer
{

void Bookmark::serialize(const ::Bookmark& bookmark, QJsonObject &json)
{
    json["line"] = static_cast<int>(bookmark.line_number_);
    json["text"] = bookmark.text_;
    json["icon"] = bookmark.icon_;
}

void Bookmark::deserialize(::Bookmark& bookmark, const QJsonObject &json)
{
    (void) bookmark;
    (void) json;
}

}  // namespace serializer
