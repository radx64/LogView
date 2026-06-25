#include "AutoBookmark.hpp"

#include <QRegularExpression>

AutoBookmark::AutoBookmark(const QString& name, const QString& description,
                           const QString& pattern, const QString& icon,
                           const QStringList& tags,
                           bool enabled, bool caseSensitive)
    : name_(name),
      description_(description),
      pattern_(pattern),
      icon_(icon),
      tags_(tags),
      enabled_(enabled),
      case_sensitive_(caseSensitive)
{
}

QRegularExpression AutoBookmark::regularExpression() const
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!case_sensitive_)
        options |= QRegularExpression::CaseInsensitiveOption;
    return QRegularExpression(pattern_, options);
}
