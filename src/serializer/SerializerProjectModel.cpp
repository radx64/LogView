#include "SerializerProjectModel.hpp"

#include <QJsonObject>
#include <QJsonArray>

#include "../ProjectModel.hpp"
#include "SerializerLogfile.hpp"
#include "../Logfile.hpp"

namespace serializer
{

void ProjectModel::serialize(const ::ProjectModel &pm, QJsonObject &json)
{
    QJsonArray array;

    for (const auto& logfile : pm.logfiles_)
    {
        QJsonObject jsonLogfile;
        Logfile::serialize(*logfile, jsonLogfile);
        array.append(jsonLogfile);
    }

    json["logfiles"] = array;
    json["projectName"] = pm.projectName_;
}
void ProjectModel::deserialize(::ProjectModel &pm, const QJsonObject &json)
{
    QJsonArray logfiles = json["logfiles"].toArray();
    pm.projectName_ = json["projectName"].toString();

    for (const QJsonValue child : logfiles)
    {
         //filepath should be read inside of Logfile deserialize
         std::unique_ptr<::Logfile> logfile = std::make_unique<::Logfile>(child["filepath"].toString());
         ::serializer::Logfile::deserialize(*logfile, child.toObject());
         pm.add_to_project(std::move(logfile));
    }
    pm.changed_ = false;
}

}  // namespace serializer
