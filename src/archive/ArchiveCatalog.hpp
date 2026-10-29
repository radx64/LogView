#pragma once

#include <functional>

#include <QString>
#include <QStringList>

struct ArchiveCatalogResult
{
    QStringList entry_paths;
    bool truncated = false;
};

class ArchiveCatalog
{
public:
    using ProgressCallback = std::function<bool(int entries_found)>;

    static ArchiveCatalogResult build(const QString& archive_path,
                                      const ProgressCallback& on_progress = {});

    static bool extractNestedEntry(const QString& archive_path,
                                   const QString& entry_id,
                                   const QString& dest_path,
                                   QString* error = nullptr);
};
