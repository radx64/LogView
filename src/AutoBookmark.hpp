#pragma once

#include <QString>
#include <QStringList>

class QRegularExpression;

class AutoBookmark
{
public:
    AutoBookmark() = default;
    AutoBookmark(const QString& name, const QString& description,
                 const QString& pattern, const QString& icon,
                 const QStringList& tags = QStringList(),
                 bool enabled = true, bool caseSensitive = true);

    QRegularExpression regularExpression() const;

    QString name_{};
    QString description_{};
    QString pattern_{};
    QString icon_{};
    QStringList tags_{};
    bool enabled_{true};
    bool case_sensitive_{true};
};
