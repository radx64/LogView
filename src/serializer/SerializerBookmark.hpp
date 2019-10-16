#ifndef SERIALIZER_SERIALIZERBOOKMARK_HPP
#define SERIALIZER_SERIALIZERBOOKMARK_HPP

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

#endif // SERIALIZER_SERIALIZERBOOKMARK_HPP
