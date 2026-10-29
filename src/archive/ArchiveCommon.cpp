#include "ArchiveCommon.hpp"

#include <QFileInfo>

namespace logview_archive
{

QString normalizeEntryName(const QString& name)
{
    QString n = name;
    n.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (n.startsWith(QLatin1Char('/')))
        n.remove(0, 1);
    return n;
}

bool isArchiveFileName(const QString& file_name)
{
    const QString lower = file_name.toLower();
    return lower.endsWith(QStringLiteral(".tar.gz"))
        || lower.endsWith(QStringLiteral(".tar.xz"))
        || lower.endsWith(QStringLiteral(".tgz"))
        || lower.endsWith(QStringLiteral(".txz"))
        || lower.endsWith(QStringLiteral(".zip"))
        || lower.endsWith(QStringLiteral(".xz"))
        || lower.endsWith(QStringLiteral(".gz"));
}

bool isArchivePath(const QString& path)
{
    return isArchiveFileName(QFileInfo(path).fileName());
}

bool isNestedArchiveLeafName(const QString& entry_name)
{
    const QString lower = normalizeEntryName(entry_name).toLower();
    return lower.endsWith(QStringLiteral(".tar.gz"))
        || lower.endsWith(QStringLiteral(".tar.xz"))
        || lower.endsWith(QStringLiteral(".tgz"))
        || lower.endsWith(QStringLiteral(".txz"))
        || lower.endsWith(QStringLiteral(".zip"))
        || lower.endsWith(QStringLiteral(".xz"))
        || lower.endsWith(QStringLiteral(".gz"));
}

std::optional<NestedPathSplit> splitNestedEntryPath(const QString& entry_path)
{
    const QString normalized = normalizeEntryName(entry_path);
    struct Marker
    {
        const char* text;
        int marker_len;
    };
    static const Marker markers[] = {
        {".tar.gz/", 8},
        {".tar.xz/", 8},
        {".tgz/", 5},
        {".txz/", 5},
        {".zip/", 5},
        {".xz/", 4},
        {".gz/", 4},
    };

    int best_idx = -1;
    int best_marker_len = 0;
    for (const Marker& marker : markers)
    {
        const QString needle = QString::fromLatin1(marker.text);
        const int idx = normalized.indexOf(needle, 0, Qt::CaseInsensitive);
        if (idx < 0)
            continue;

        if (best_idx < 0 || idx < best_idx
            || (idx == best_idx && marker.marker_len > best_marker_len))
        {
            best_idx = idx;
            best_marker_len = marker.marker_len;
        }
    }

    if (best_idx < 0)
        return std::nullopt;

    NestedPathSplit split;
    split.outer_entry = normalized.left(best_idx + best_marker_len - 1);
    split.inner_path = normalized.mid(best_idx + best_marker_len);
    return split;
}

}  // namespace logview_archive
