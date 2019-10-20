#ifndef GREP_NODE_HPP
#define GREP_NODE_HPP

#include <memory>
#include <string>
#include <vector>

namespace serializer { class GrepNode; }

class GrepNode
{
public:
    GrepNode(
        std::string value,
        bool is_regex = false,
        bool is_case_insensitive = false,
        bool is_inverted = false);

    GrepNode() = default;

    ~GrepNode();

    std::string getPattern() const;

    bool isRegEx() const;

    bool isCaseInsensitive() const;

    bool isInverted() const;

    void addChild(GrepNode* node);

    void removeChild(GrepNode* node);

    std::vector<GrepNode*> getChildren() const;

protected:
    std::vector<GrepNode*> children_;
    std::string pattern_;
    bool is_regex_;
    bool is_case_insensitive_;
    bool is_inverted_;

    friend class serializer::GrepNode;
};

#endif // GREP_NODE_HPP
