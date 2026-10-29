#include "SerializerArchiveDocument.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include "../ArchiveDocument.hpp"
#include "../Logfile.hpp"
#include "SerializerLogfile.hpp"

namespace serializer
{

void ArchiveDocument::serialize(const ::ArchiveDocument& archive, QJsonObject& json)
{
    json["filepath"] = archive.archive_path_;
    QJsonArray inner_files;
    for (const ::ArchiveDocument::OpenedEntry& entry : archive.opened_)
    {
        QJsonObject inner;
        inner["entryPath"] = entry.entry_id;
        serializer::Logfile::serialize(*entry.logfile, inner);
        inner.remove("filepath");
        inner_files.append(inner);
    }
    json["innerFiles"] = inner_files;
}

void ArchiveDocument::deserialize(::ArchiveDocument& archive, const QJsonObject& json)
{
    archive.archive_path_ = json["filepath"].toString();
    const QJsonArray inner_files = json["innerFiles"].toArray();
    for (const QJsonValue& value : inner_files)
    {
        const QJsonObject obj = value.toObject();
        const QString entry_path = obj["entryPath"].toString();
        if (entry_path.isEmpty())
            continue;
        archive.enqueuePendingRestore(entry_path, obj);
    }
}

}  // namespace serializer
