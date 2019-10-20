#ifndef GREP_NODE_HPP
#define GREP_NODE_HPP

#include <memory>
#include <string>
#include <vector>

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

namespace serializer { class GrepNode; }

class GrepNode
{
public:
GrepNode(
    std::string value,
    bool is_regex = false,
    bool is_case_insensitive = false,
    bool is_inverted = false)
: pattern_{value},
    is_regex_{is_regex},
    is_case_insensitive_{is_case_insensitive},
    is_inverted_{is_inverted}
{}

GrepNode() = default;

~GrepNode()
{
    for (auto child : children_) delete child;
}

std::string getPattern() const
{
    return pattern_;
}

bool isRegEx() const
{
    return is_regex_;
}

bool isCaseInsensitive() const
{
    return is_case_insensitive_;
}

bool isInverted() const
{
    return is_inverted_;
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

std::vector<GrepNode*> getChildren() const
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
    result += QString(pattern_.c_str());
    qDebug() << result;

    for (auto& element : children_)
    {
        element->evaluate(level+1);
    }
}

protected:
    std::vector<GrepNode*> children_;
    std::string pattern_;
    bool is_regex_;
    bool is_case_insensitive_;
    bool is_inverted_;

    friend class serializer::GrepNode;
};

#endif // GREP_NODE_HPP
