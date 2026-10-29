#pragma once

#include <memory>
#include <vector>

#include <QJsonObject>
#include <QObject>
#include <QStringList>

#include "archive/ArchiveExtractionCache.hpp"

class Logfile;

namespace serializer { class ArchiveDocument; }

class ArchiveDocument : public QObject
{
Q_OBJECT
public:
    explicit ArchiveDocument(const QString& archive_path);
    ~ArchiveDocument() override;

    const QString& archivePath() const;
    const QStringList& catalogEntryPaths() const;
    bool catalogTruncated() const;
    bool catalogReady() const;

    ArchiveExtractionCache* extractionCache();

    struct OpenedEntry
    {
        QString entry_id;
        std::unique_ptr<Logfile> logfile;
    };

    struct PendingInnerRestore
    {
        QString entry_id;
        QJsonObject state;
    };

    const std::vector<OpenedEntry>& openedEntries() const { return opened_; }
    const std::vector<PendingInnerRestore>& pendingRestores() const { return pending_restores_; }
    void clearPendingRestores() { pending_restores_.clear(); }

    Logfile* findOpenedLogfile(const QString& entry_id) const;

    Logfile* openEntry(const QString& entry_id, QString* error = nullptr);
    void closeEntry(const QString& entry_id);

    void enqueuePendingRestore(const QString& entry_id, const QJsonObject& state);
    bool hasPendingRestore() const;

    void setCatalog(const QStringList& paths, bool truncated);

protected:
    friend class serializer::ArchiveDocument;

    QString archive_path_;
    QStringList catalog_paths_;
    bool catalog_truncated_ = false;
    bool catalog_ready_ = false;
    std::unique_ptr<ArchiveExtractionCache> cache_;

    std::vector<OpenedEntry> opened_;
    std::vector<PendingInnerRestore> pending_restores_;

signals:
    void changed();
    void catalogUpdated();
};
