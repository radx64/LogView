#pragma once

class QJsonObject;
class MarkingsModel;

namespace serializer
{

class MarkingsModel
{
public:
    MarkingsModel() = delete;
    static void serialize(const ::MarkingsModel& mmodel, QJsonObject &json);
    static void deserialize(::MarkingsModel& mmodel, const QJsonObject &json);
};

}  // namespace serializer
