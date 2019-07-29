#ifndef GREP_NODE_HPP
#define GREP_NODE_HPP

#include <memory>
#include <string>
#include <vector>

class GrepNode
{
public:
    GrepNode() : value_{}
    {}

    std::string getValue()
    {
        return value_;
    }

    void addChild(std::unique_ptr<GrepNode> node)
    {
        children_.push_back(std::move(node));
    }

    std::vector<GrepNode*> getChildren()
    {
        std::vector<GrepNode*> result(children_.size());
        for (const auto& child : children_) result.push_back(child.get());
        return result;
    }

protected:
    std::vector<std::unique_ptr<GrepNode>> children_;
    std::string value_{};
};

#endif // GREP_NODE_HPP
