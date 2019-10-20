#include "SerializerGrepNode.hpp"

#include "../GrepNode.hpp"

#include <QJsonObject>
#include <QJsonArray>

namespace serializer
{

void GrepNode::serialize(const ::GrepNode &gp, QJsonObject &json)
{
    json["pattern"] = QString::fromStdString(gp.pattern_);
    json["is_regex"] = gp.is_regex_;
    json["is_case_insensitive"] = gp.is_case_insensitive_;
    json["is_inverted"] = gp.is_inverted_;

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
    gp.pattern_ = json["pattern"].toString().toStdString();
    gp.is_regex_ = json["is_regex"].toBool();
    gp.is_case_insensitive_= json["is_case_insensitive"].toBool();
    gp.is_inverted_= json["is_inverted"].toBool();

    QJsonArray children = json["childern"].toArray();
    for (const QJsonValue child : children)
    {
        ::GrepNode *c = new ::GrepNode;
        serializer::GrepNode::deserialize(*c, child.toObject());
        gp.children_.push_back(c);
    }
}

}  // namespace serializer

