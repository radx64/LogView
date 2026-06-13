#include "SerializerMarkingsModel.hpp"

#include "../MarkingsModel.hpp"
#include "SerializerMarking.hpp"

#include <QJsonArray>
#include <QJsonObject>

namespace serializer
{
void MarkingsModel::serialize(const ::MarkingsModel& mmodel, QJsonObject &json)
{
    QJsonArray array;
    for (const auto& marking : mmodel.markings_)
    {
        QJsonObject jsonMarking;
        Marking::serialize(marking, jsonMarking);
        array.append(jsonMarking);
    }
    json["markings"] = array;
}

void MarkingsModel::deserialize(::MarkingsModel& mmodel, const QJsonObject &json)
{
    QJsonArray markings = json["markings"].toArray();

    for (const QJsonValue child : markings)
    {
        ::Marking m;
        ::serializer::Marking::deserialize(m, child.toObject());
        mmodel.markings_.append(m);
    }
}

}  // namespace serializer
