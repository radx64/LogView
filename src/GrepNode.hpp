#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QObject>

namespace serializer { class GrepNode; }

class GrepNode : public QObject
{
Q_OBJECT
public:
    GrepNode(
        const std::string& value,
        const bool& is_regex = false,
        const bool& is_case_sensitive = false,
        const bool& is_inverted = false);

    GrepNode() = default;

    virtual ~GrepNode();

    std::string getPattern() const;

    bool isRegEx() const;

    bool isCaseSensitive() const;

    bool isInverted() const;

    void addChild(GrepNode* node);

    void removeChild(GrepNode* node);

    std::vector<GrepNode*> getChildren() const;

protected:
    std::vector<GrepNode*> children_{};
    std::string pattern_{};
    bool is_regex_{};
    bool is_case_sensitive_{};
    bool is_inverted_{};

    friend class serializer::GrepNode;

private slots:
    void child_changed();

signals:
    void changed();
};
