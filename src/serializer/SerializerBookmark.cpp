#include "SerializerBookmark.hpp"

#include "../Bookmark.hpp"

#include <QJsonObject>

#include <QDebug>

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
    bookmark.line_number_ = static_cast<uint32_t>(json["line"].toInt());
    bookmark.text_ = json["text"].toString();
    bookmark.icon_ = json["icon"].toString();
}

}  // namespace serializer
