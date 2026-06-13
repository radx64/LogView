#pragma once

class ProjectModel;
class QJsonObject;

namespace serializer
{

class ProjectModel
{
public:
    ProjectModel() = delete;
    static void serialize(const ::ProjectModel &pm, QJsonObject &json);
    static void deserialize(::ProjectModel &pm, const QJsonObject &json);
};

}  // namespace serializer
