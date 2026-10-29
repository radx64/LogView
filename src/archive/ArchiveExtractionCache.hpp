#pragma once

#include <QHash>
#include <QString>

class ArchiveExtractionCache
{
public:
    explicit ArchiveExtractionCache(const QString& archive_path);

    QString archivePath() const { return archive_path_; }

    QString ensureExtracted(const QString& entry_id, QString* error = nullptr);

    void clear();

private:
    QString cacheRoot() const;
    QString cacheKey() const;

    QString archive_path_;
    QHash<QString, QString> extracted_;
};
