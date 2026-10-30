#include "ArchivePathFilter.hpp"

namespace logview_archive
{
namespace
{

bool pathMatchesPattern(const QString& needle, const QString& path)
{
    if (needle.isEmpty())
        return true;

    const QString haystack = path.toCaseFolded();
    qsizetype start = 0;
    while (start <= haystack.size())
    {
        const qsizetype slash = haystack.indexOf(QLatin1Char('/'), start);
        const QString segment = slash < 0 ? haystack.mid(start) : haystack.mid(start, slash - start);
        if (segment.contains(needle))
            return true;
        if (slash < 0)
            break;
        start = slash + 1;
    }
    return false;
}

}  // namespace

QStringList filterEntryPaths(const QStringList& paths, const QString& pattern)
{
    const QString needle = pattern.trimmed().toCaseFolded();
    if (needle.isEmpty())
        return paths;

    QStringList filtered;
    filtered.reserve(paths.size() / 4);
    for (const QString& path : paths)
    {
        if (pathMatchesPattern(needle, path))
            filtered.append(path);
    }
    return filtered;
}

}  // namespace logview_archive
