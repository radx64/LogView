#ifndef GREP_NODE_HPP
#define GREP_NODE_HPP

#include <memory>
#include <string>
#include <vector>

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

#include "Serializable.hpp"

class GrepNode : public Serializable
{
public:
GrepNode(std::string value) : value_{value}
{
}

~GrepNode() override
{
    for (auto child : children_) delete child;
}

std::string getValue()
{
    return value_;
}

void addChild(GrepNode* node)
{
    children_.push_back(std::move(node));
}

void removeChild(GrepNode* node)
{
    auto it = std::find(children_.begin(), children_.end(), node);
    if (it == children_.end()) return;
    children_.erase(it);
    delete node;
}

std::vector<GrepNode*> getChildren()
{
    std::vector<GrepNode*> result;
    result.reserve(children_.size());
    for (const auto& child : children_) result.push_back(child);
    return result;
}

//DEBUG check how tree is beeing created
void evaluate(const unsigned char level)
{
    QString result;
    for (unsigned char i=0; i < level; ++i)
    {
        result += "-";
    }
    result += QString(value_.c_str());
    qDebug() << result;

    for (auto& element : children_)
    {
        element->evaluate(level+1);
    }
}

virtual void serialize(QJsonObject &json) const override
{
    json["pattern"] = QString::fromStdString(value_);
    QJsonArray array;
    for (const auto& child : children_)
    {
        QJsonObject json_child;
        child->serialize(json_child);
        array.append(json_child);
    }
   json["childs"] = array;
}

virtual void deserialize(const QJsonObject &json) override
{
    (void)json;
}

protected:
    std::vector<GrepNode*> children_;
    std::string value_{};
};

#endif // GREP_NODE_HPP
