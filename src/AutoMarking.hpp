#pragma once

#include <QString>

class QRegularExpression;

class AutoMarking
{
public:
    AutoMarking() = default;
    AutoMarking(const QString& pattern, const QString& textColor,
                const QString& backgroundColor = QString(),
                bool enabled = true, bool caseSensitive = true);

    QRegularExpression regularExpression() const;

    QString pattern_{};
    QString text_color_{};
    QString background_color_{};
    bool enabled_{true};
    bool case_sensitive_{true};
};
