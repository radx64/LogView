#include "SerializerGrepNode.hpp"

#include "../GrepNode.hpp"

#include <QJsonObject>
#include <QJsonArray>

namespace serializer
{

void GrepNode::serialize(const ::GrepNode &gp, QJsonObject &json)
{
    json["pattern"] = QString::fromStdString(gp.value_);
    QJsonArray array;
    for (const auto& child : gp.children_)
    {
        QJsonObject json_child;
        GrepNode::serialize(*child, json_child);
        array.append(json_child);
    }
   json["childern"] = array;
}

void GrepNode::deserialize(::GrepNode &gp, const QJsonObject &json)
{
    (void)gp;
    (void)json;
}

}  // namespace serializer

