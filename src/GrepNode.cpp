#include "GrepNode.hpp"

#include <memory>
#include <string>
#include <vector>

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

GrepNode::GrepNode(
    std::string value,
    bool is_regex,
    bool is_case_insensitive,
    bool is_inverted)
: pattern_{value},
    is_regex_{is_regex},
    is_case_insensitive_{is_case_insensitive},
    is_inverted_{is_inverted}
{}

GrepNode::~GrepNode()
{
    for (auto child : children_)
    {
        QObject::disconnect(child, &GrepNode::changed,
                         this, &GrepNode::child_changed);
        delete child;
    }
}

std::string GrepNode::getPattern() const
{
    return pattern_;
}

bool GrepNode::isRegEx() const
{
    return is_regex_;
}

bool GrepNode::isCaseInsensitive() const
{
    return is_case_insensitive_;
}

bool GrepNode::isInverted() const
{
    return is_inverted_;
}

void GrepNode::addChild(GrepNode* node)
{
    children_.push_back(std::move(node));
    auto added_child = children_.back();
    QObject::connect(added_child, &GrepNode::changed,
                     this, &GrepNode::child_changed);

    emit changed();
}

void GrepNode::removeChild(GrepNode* node)
{
    auto it = std::find(children_.begin(), children_.end(), node);
    if (it == children_.end()) return;
    children_.erase(it);
    QObject::disconnect(node, &GrepNode::changed,
                        this, &GrepNode::child_changed);
    delete node;
}

std::vector<GrepNode*> GrepNode::getChildren() const
{
    std::vector<GrepNode*> result;
    result.reserve(children_.size());
    for (const auto& child : children_) result.push_back(child);
    return result;
}

void GrepNode::child_changed()
{
  emit changed();
}
