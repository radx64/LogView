#pragma once

#include <optional>

#include <QString>

namespace logview_archive
{

constexpr int kMaxDepth = 16;
constexpr int kMaxEntries = 50000;

QString normalizeEntryName(const QString& name);

bool isArchiveFileName(const QString& file_name);
bool isArchivePath(const QString& path);

bool isNestedArchiveLeafName(const QString& entry_name);

struct NestedPathSplit
{
    QString outer_entry;
    QString inner_path;
};

std::optional<NestedPathSplit> splitNestedEntryPath(const QString& entry_path);

}  // namespace logview_archive
