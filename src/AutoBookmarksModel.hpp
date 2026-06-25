#pragma once

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include "AutoBookmark.hpp"

class AutoBookmarksModel : public QObject
{
    Q_OBJECT
public:
    static AutoBookmarksModel& instance();
    static QString dirPath();

    void load();

    QStringList files() const;
    QVector<AutoBookmark> rules(const QString& file) const;
    AutoBookmark rule(const QString& file, int row) const;
    void addRule(const QString& file, const AutoBookmark& rule);
    void updateRule(const QString& file, int row, const AutoBookmark& rule);
    void removeRule(const QString& file, int row);
    QString createFile(const QString& name);
    void deleteFile(const QString& file);

    QStringList allTags() const;
    bool isTagEnabled(const QString& tag) const;
    bool isFileEnabled(const QString& file) const;
    void setTagEnabled(const QString& tag, bool enabled);
    void setFileEnabled(const QString& file, bool enabled);

    QVector<AutoBookmark> enabledRules() const;

signals:
    void changed();
    void filesChanged();

private:
    explicit AutoBookmarksModel(QObject* parent = nullptr);
    AutoBookmarksModel(const AutoBookmarksModel&) = delete;
    AutoBookmarksModel& operator=(const AutoBookmarksModel&) = delete;

    void saveFile(const QString& file) const;
    void ensureExampleFile();
    void loadFilterState();

    QMap<QString, QVector<AutoBookmark>> files_{};
    QSet<QString> disabled_tags_{};
    QSet<QString> disabled_files_{};
};
