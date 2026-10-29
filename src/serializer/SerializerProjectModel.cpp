#include "SerializerProjectModel.hpp"

#include <QJsonObject>
#include <QJsonArray>

#include "../ProjectModel.hpp"
#include "../ArchiveDocument.hpp"
#include "SerializerLogfile.hpp"
#include "SerializerArchiveDocument.hpp"
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

    QJsonArray archives;
    for (const auto& archive : pm.archives_)
    {
        QJsonObject json_archive;
        ArchiveDocument::serialize(*archive, json_archive);
        archives.append(json_archive);
    }
    json["archives"] = archives;

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

    if (json.contains("archives"))
    {
        const QJsonArray archives = json["archives"].toArray();
        for (const QJsonValue& value : archives)
        {
            const QJsonObject obj = value.toObject();
            auto archive = std::make_unique<::ArchiveDocument>(obj["filepath"].toString());
            ::serializer::ArchiveDocument::deserialize(*archive, obj);
            pm.add_archive_to_project(std::move(archive));
        }
    }
}

}  // namespace serializer
