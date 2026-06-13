#pragma once

class QJsonObject;
class Bookmark;

namespace serializer
{

class Bookmark
{
public:
    Bookmark() = delete;
    static void serialize(const ::Bookmark& bookmark, QJsonObject &json);
    static void deserialize(::Bookmark& bookmark, const QJsonObject &json) ;
protected:
};

}  // namespace serialzer
