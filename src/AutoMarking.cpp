#include "AutoMarking.hpp"

#include <QRegularExpression>

AutoMarking::AutoMarking(const QString& pattern, const QString& textColor,
                         const QString& backgroundColor,
                         bool enabled, bool caseSensitive)
    : pattern_(pattern),
      text_color_(textColor),
      background_color_(backgroundColor),
      enabled_(enabled),
      case_sensitive_(caseSensitive)
{
}

QRegularExpression AutoMarking::regularExpression() const
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!case_sensitive_)
        options |= QRegularExpression::CaseInsensitiveOption;
    return QRegularExpression(pattern_, options);
}
