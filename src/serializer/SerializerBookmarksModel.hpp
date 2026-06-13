#pragma once

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