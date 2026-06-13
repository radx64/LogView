#include "SerializerMarking.hpp"

#include "../Marking.hpp"

#include <QJsonObject>

namespace serializer
{

void Marking::serialize(const ::Marking& marking, QJsonObject &json)
{
    json["text"] = marking.text_;
    json["color"] = marking.color_;
}

void Marking::deserialize(::Marking& marking, const QJsonObject &json)
{
    marking.text_ = json["text"].toString();
    marking.color_ = json["color"].toString();
    if (marking.color_.isEmpty())
        marking.color_ = QString::fromLatin1(marking_colors::kDefaultColor);
}

}  // namespace serializer
