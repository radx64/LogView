#ifndef SERIALIZER_SERIALIZERBOOKMARKS_MODEL_HPP
#define SERIALIZER_SERIALIZERBOOKMARKS_MODEL_HPP

class QJsonObject;
class BookmarksModel;

namespace serializer
{

class BookmarksModel
{
public:
    BookmarksModel() = delete;
    static void serialize(const ::BookmarksModel& bmodel, QJsonObject &json);
    static void deserialize(::BookmarksModel& bmodel, const QJsonObject &json);
};

}  // namespace serializer
#endif // SERIALIZER_SERIALIZERBOOKMARKS_MODEL_HPP
