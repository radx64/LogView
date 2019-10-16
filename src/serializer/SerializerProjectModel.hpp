#ifndef SERIALIZER_SERIALIZERPROJECT_MODEL_HPP
#define SERIALIZER_SERIALIZERPROJECT_MODEL_HPP

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

#endif // SERIALIZER_SERIALIZERPROJECT_MODEL_HPP
