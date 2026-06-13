#pragma once

class QJsonObject;
class Marking;

namespace serializer
{

class Marking
{
public:
    Marking() = delete;
    static void serialize(const ::Marking& marking, QJsonObject &json);
    static void deserialize(::Marking& marking, const QJsonObject &json);
};

}  // namespace serializer
