#pragma once

class ArchiveDocument;
class QJsonObject;

namespace serializer
{

class ArchiveDocument
{
public:
    ArchiveDocument() = delete;
    static void serialize(const ::ArchiveDocument& archive, QJsonObject& json);
    static void deserialize(::ArchiveDocument& archive, const QJsonObject& json);
};

}  // namespace serializer
