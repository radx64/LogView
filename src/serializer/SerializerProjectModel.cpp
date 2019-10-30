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
        Logfile::serialize(*logfile.get(), jsonLogfile);
        array.append(jsonLogfile);
    }

    json["logfiles"] = array;
}
void ProjectModel::deserialize(::ProjectModel &pm, const QJsonObject &json)
{
    QJsonArray logfiles = json["logfiles"].toArray();

    for (const QJsonValue child : logfiles)
    {
         std::unique_ptr<::Logfile> logfile = std::make_unique<::Logfile>(child["filepath"].toString());
         ::serializer::Logfile::deserialize(*logfile.get(), child.toObject());
         pm.logfiles_.push_back(std::move(logfile));
    }
}

}  // namespace serializer
