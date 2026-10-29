#include "ArchiveExtractionCache.hpp"

#include "ArchiveCatalog.hpp"
#include "ArchiveCommon.hpp"
#include "Translator.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

ArchiveExtractionCache::ArchiveExtractionCache(const QString& archive_path)
    : archive_path_(QFileInfo(archive_path).absoluteFilePath())
{
}

QString ArchiveExtractionCache::cacheRoot() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/LogView/archives");
}

QString ArchiveExtractionCache::cacheKey() const
{
    QFileInfo info(archive_path_);
    const QByteArray material = archive_path_.toUtf8()
        + QByteArray::number(info.size())
        + QByteArray::number(info.lastModified().toSecsSinceEpoch());
    return QString::fromLatin1(QCryptographicHash::hash(material, QCryptographicHash::Sha1).toHex());
}

QString ArchiveExtractionCache::ensureExtracted(const QString& entry_id, QString* error)
{
    const QString key = logview_archive::normalizeEntryName(entry_id);
    if (extracted_.contains(key))
        return extracted_.value(key);

    QDir dir(cacheRoot() + QLatin1Char('/') + cacheKey());
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath()))
    {
        if (error)
            *error = Lang::tr("archive.error.extract_cache_dir");
        return QString();
    }

    const QString safe_name = QString::fromLatin1(
        QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex());
    const QString dest = dir.absoluteFilePath(safe_name);

    if (QFileInfo::exists(dest))
    {
        extracted_.insert(key, dest);
        return dest;
    }

    if (!ArchiveCatalog::extractNestedEntry(archive_path_, key, dest, error))
        return QString();

    extracted_.insert(key, dest);
    return dest;
}

void ArchiveExtractionCache::clear()
{
    extracted_.clear();
    QDir dir(cacheRoot() + QLatin1Char('/') + cacheKey());
    if (dir.exists())
        dir.removeRecursively();
}
