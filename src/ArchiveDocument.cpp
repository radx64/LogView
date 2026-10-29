#include "ArchiveDocument.hpp"

#include "Logfile.hpp"

#include <algorithm>

#include <QFileInfo>

ArchiveDocument::ArchiveDocument(const QString& archive_path)
    : archive_path_(QFileInfo(archive_path).absoluteFilePath())
    , cache_(std::make_unique<ArchiveExtractionCache>(archive_path_))
{
}

ArchiveDocument::~ArchiveDocument()
{
    if (cache_)
        cache_->clear();
}

const QString& ArchiveDocument::archivePath() const
{
    return archive_path_;
}

const QStringList& ArchiveDocument::catalogEntryPaths() const
{
    return catalog_paths_;
}

bool ArchiveDocument::catalogTruncated() const
{
    return catalog_truncated_;
}

bool ArchiveDocument::catalogReady() const
{
    return catalog_ready_;
}

ArchiveExtractionCache* ArchiveDocument::extractionCache()
{
    return cache_.get();
}

void ArchiveDocument::setCatalog(const QStringList& paths, bool truncated)
{
    catalog_paths_ = paths;
    catalog_truncated_ = truncated;
    catalog_ready_ = true;
    emit catalogUpdated();
}

Logfile* ArchiveDocument::findOpenedLogfile(const QString& entry_id) const
{
    for (const OpenedEntry& entry : opened_)
    {
        if (entry.entry_id == entry_id)
            return entry.logfile.get();
    }
    return nullptr;
}

Logfile* ArchiveDocument::openEntry(const QString& entry_id, QString* error)
{
    if (Logfile* existing = findOpenedLogfile(entry_id))
        return existing;

    QString extract_error;
    const QString temp_path = cache_->ensureExtracted(entry_id, &extract_error);
    if (temp_path.isEmpty())
    {
        if (error) *error = extract_error;
        return nullptr;
    }

    const QString archive_name = QFileInfo(archive_path_).fileName();
    const QString display = archive_name + QStringLiteral(" \u203A ") + entry_id;

    OpenedEntry entry;
    entry.entry_id = entry_id;
    entry.logfile = std::make_unique<Logfile>(temp_path, display);
    Logfile* lf = entry.logfile.get();
    opened_.push_back(std::move(entry));

    connect(lf, &Logfile::changed, this, &ArchiveDocument::changed);
    emit changed();
    return lf;
}

void ArchiveDocument::closeEntry(const QString& entry_id)
{
    auto it = std::find_if(opened_.begin(), opened_.end(),
                           [&entry_id](const OpenedEntry& e) { return e.entry_id == entry_id; });
    if (it == opened_.end())
        return;
    opened_.erase(it);
    emit changed();
}

void ArchiveDocument::enqueuePendingRestore(const QString& entry_id, const QJsonObject& state)
{
    pending_restores_.push_back(PendingInnerRestore{entry_id, state});
}

bool ArchiveDocument::hasPendingRestore() const
{
    return !pending_restores_.empty();
}
