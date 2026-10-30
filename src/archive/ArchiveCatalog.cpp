#include "ArchiveCatalog.hpp"

#include "ArchiveCommon.hpp"
#include "Translator.hpp"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>

#include <archive.h>
#include <archive_entry.h>

namespace
{

struct ArchiveHandle
{
    archive* reader = nullptr;

    ~ArchiveHandle()
    {
        close();
    }

    void close()
    {
        if (!reader)
            return;
        archive_read_close(reader);
        archive_read_free(reader);
        reader = nullptr;
    }
};

bool openArchiveFromPath(const QString& path, ArchiveHandle& handle, QString* error)
{
    handle.reader = archive_read_new();
    archive_read_support_filter_all(handle.reader);
    archive_read_support_format_all(handle.reader);
    archive_read_support_format_raw(handle.reader);

    if (archive_read_open_filename(handle.reader, path.toUtf8().constData(), 10240) != ARCHIVE_OK)
    {
        if (error)
            *error = QString::fromUtf8(archive_error_string(handle.reader));
        handle.close();
        return false;
    }
    return true;
}

bool openArchiveFromMemory(QByteArray& data, ArchiveHandle& handle, QString* error)
{
    handle.reader = archive_read_new();
    archive_read_support_filter_all(handle.reader);
    archive_read_support_format_all(handle.reader);
    archive_read_support_format_raw(handle.reader);

    if (archive_read_open_memory(handle.reader, data.data(), static_cast<size_t>(data.size())) != ARCHIVE_OK)
    {
        if (error)
            *error = QString::fromUtf8(archive_error_string(handle.reader));
        handle.close();
        return false;
    }
    return true;
}

bool entryIsDirectory(archive_entry* entry)
{
    return archive_entry_filetype(entry) == AE_IFDIR;
}

QString entryPath(archive_entry* entry)
{
    return logview_archive::normalizeEntryName(QString::fromUtf8(archive_entry_pathname(entry)));
}

QString formatMemberMissingError(const QString& member,
                                 const QString& container_label,
                                 const QString& full_logical_path)
{
    QString message = Lang::tr("archive.error.extract_member_missing")
                          .arg(member, container_label);
    if (!full_logical_path.isEmpty())
    {
        message += QLatin1Char('\n')
                   + Lang::tr("archive.error.extract_full_path").arg(full_logical_path);
    }
    message += QLatin1Char('\n') + Lang::tr("archive.error.extract_hint");
    return message;
}

bool readCurrentEntryToBuffer(ArchiveHandle& handle, QByteArray& out, QString* error)
{
    out.clear();
    char buffer[8192];
    while (true)
    {
        const la_ssize_t read = archive_read_data(handle.reader, buffer, sizeof(buffer));
        if (read == 0)
            break;
        if (read < 0)
        {
            if (error)
                *error = QString::fromUtf8(archive_error_string(handle.reader));
            out.clear();
            return false;
        }
        out.append(buffer, static_cast<int>(read));
    }
    return true;
}

bool locateAndReadEntry(ArchiveHandle& handle,
                        const QString& entry_name,
                        QByteArray& out,
                        const QString& container_label,
                        const QString& full_logical_path,
                        QString* error)
{
    const QString needle = logview_archive::normalizeEntryName(entry_name);
    archive_entry* entry = nullptr;

    while (archive_read_next_header(handle.reader, &entry) == ARCHIVE_OK)
    {
        if (entryIsDirectory(entry))
            continue;

        if (entryPath(entry) != needle && !entryPath(entry).isEmpty())
        {
            archive_read_data_skip(handle.reader);
            continue;
        }

        return readCurrentEntryToBuffer(handle, out, error);
    }

    if (error)
    {
        const char* lib_err = archive_error_string(handle.reader);
        if (lib_err && lib_err[0] != '\0')
        {
            *error = Lang::tr("archive.error.extract_read_while_seek")
                         .arg(container_label, QString::fromUtf8(lib_err));
            *error += QLatin1Char('\n')
                      + Lang::tr("archive.error.extract_full_path").arg(full_logical_path);
        }
        else
        {
            *error = formatMemberMissingError(needle, container_label, full_logical_path);
        }
    }
    return false;
}

bool isSingleStreamCompressed(const QString& entry_name)
{
    const QString lower = entry_name.toLower();
    if (lower.endsWith(QStringLiteral(".tar.gz"))
        || lower.endsWith(QStringLiteral(".tar.xz"))
        || lower.endsWith(QStringLiteral(".tgz"))
        || lower.endsWith(QStringLiteral(".txz")))
    {
        return false;
    }
    return lower.endsWith(QStringLiteral(".xz")) || lower.endsWith(QStringLiteral(".gz"));
}

// .log.xz entries are one compressed stream with no inner path. Write the
// decompressed bytes so the log viewer opens text. Leave real archives as they
// are when the stream contains more than one entry.
bool decompressSingleStreamFile(const QString& entry_name, const QString& dest_path, QString* error)
{
    if (!isSingleStreamCompressed(entry_name))
        return true;

    QFile compressed(dest_path);
    if (!compressed.open(QIODevice::ReadOnly))
    {
        if (error)
            *error = Lang::tr("archive.error.extract_write_failed").arg(dest_path, compressed.errorString());
        return false;
    }
    QByteArray data = compressed.readAll();
    compressed.close();

    ArchiveHandle decoded;
    if (!openArchiveFromMemory(data, decoded, nullptr))
        return true;

    archive_entry* entry = nullptr;
    if (archive_read_next_header(decoded.reader, &entry) != ARCHIVE_OK)
        return true;

    QByteArray payload;
    if (!readCurrentEntryToBuffer(decoded, payload, nullptr) || payload.isEmpty())
        return true;

    archive_entry* next = nullptr;
    if (archive_read_next_header(decoded.reader, &next) == ARCHIVE_OK)
        return true;

    QFile out(dest_path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error)
            *error = Lang::tr("archive.error.extract_write_failed").arg(dest_path, out.errorString());
        return false;
    }
    if (out.write(payload) != payload.size())
    {
        if (error)
            *error = Lang::tr("archive.error.extract_write_failed").arg(dest_path, out.errorString());
        return false;
    }
    return true;
}

bool locateAndExtractEntry(ArchiveHandle& handle,
                           const QString& entry_name,
                           const QString& dest_path,
                           const QString& container_label,
                           const QString& full_logical_path,
                           QString* error)
{
    const QString needle = logview_archive::normalizeEntryName(entry_name);
    archive_entry* entry = nullptr;

    while (archive_read_next_header(handle.reader, &entry) == ARCHIVE_OK)
    {
        if (entryIsDirectory(entry))
            continue;

        if (entryPath(entry) != needle && !entryPath(entry).isEmpty())
        {
            archive_read_data_skip(handle.reader);
            continue;
        }

        QFile out(dest_path);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            if (error)
                *error = Lang::tr("archive.error.extract_write_failed").arg(dest_path, out.errorString());
            return false;
        }

        char buffer[8192];
        while (true)
        {
            const la_ssize_t read = archive_read_data(handle.reader, buffer, sizeof(buffer));
            if (read == 0)
                break;
            if (read < 0)
            {
                if (error)
                    *error = Lang::tr("archive.error.extract_read_member")
                                 .arg(needle, container_label,
                                      QString::fromUtf8(archive_error_string(handle.reader)));
                return false;
            }
            if (out.write(buffer, read) != read)
            {
                if (error)
                    *error = Lang::tr("archive.error.extract_write_failed").arg(dest_path, out.errorString());
                return false;
            }
        }
        out.close();
        return decompressSingleStreamFile(needle, dest_path, error);
    }

    if (error)
    {
        const char* lib_err = archive_error_string(handle.reader);
        if (lib_err && lib_err[0] != '\0')
        {
            *error = Lang::tr("archive.error.extract_read_while_seek")
                         .arg(container_label, QString::fromUtf8(lib_err));
            *error += QLatin1Char('\n')
                      + Lang::tr("archive.error.extract_full_path").arg(full_logical_path);
        }
        else
        {
            *error = formatMemberMissingError(needle, container_label, full_logical_path);
        }
    }
    return false;
}

bool reportCatalogProgress(const ArchiveCatalog::ProgressCallback& on_progress,
                           int entries,
                           archive* byte_source)
{
    if (!on_progress)
        return true;
    const qint64 bytes = byte_source ? archive_filter_bytes(byte_source, 0) : 0;
    return on_progress(entries, bytes);
}

void listFromHandle(ArchiveHandle& handle,
                    const QString& path_prefix,
                    int depth,
                    QStringList& out,
                    bool& truncated,
                    const ArchiveCatalog::ProgressCallback& on_progress,
                    archive* byte_source)
{
    if (depth > logview_archive::kMaxDepth)
    {
        truncated = true;
        return;
    }

    archive_entry* entry = nullptr;
    while (archive_read_next_header(handle.reader, &entry) == ARCHIVE_OK)
    {
        if (out.size() >= logview_archive::kMaxEntries)
        {
            truncated = true;
            break;
        }

        if (entryIsDirectory(entry))
        {
            archive_read_data_skip(handle.reader);
            continue;
        }

        const QString rel = entryPath(entry);
        if (rel.isEmpty())
        {
            archive_read_data_skip(handle.reader);
            continue;
        }

        const QString logical = path_prefix + rel;
        if (logview_archive::isNestedArchiveLeafName(rel))
        {
            const int count_before = out.size();
            QByteArray nested_bytes;
            ArchiveHandle nested;
            bool opened = readCurrentEntryToBuffer(handle, nested_bytes, nullptr)
                && openArchiveFromMemory(nested_bytes, nested, nullptr);

            // A plain .xz/.gz stream is one payload. libarchive names that
            // payload "data". Keep the .xz itself as the file to open.
            if (opened && isSingleStreamCompressed(rel))
            {
                archive_entry* peek = nullptr;
                bool raw_stream = true;
                if (archive_read_next_header(nested.reader, &peek) == ARCHIVE_OK)
                {
                    const int format = archive_format(nested.reader);
                    raw_stream = (format & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_RAW;
                }
                if (raw_stream)
                {
                    if (out.size() < logview_archive::kMaxEntries)
                    {
                        out.append(logical);
                        if (!reportCatalogProgress(on_progress, out.size(), byte_source))
                            break;
                    }
                    continue;
                }

                nested.close();
                opened = openArchiveFromMemory(nested_bytes, nested, nullptr);
            }

            if (opened)
            {
                listFromHandle(nested,
                               logical + QLatin1Char('/'),
                               depth + 1,
                               out,
                               truncated,
                               on_progress,
                               byte_source);
            }

            if (out.size() == count_before && out.size() < logview_archive::kMaxEntries)
            {
                out.append(logical);
                if (!reportCatalogProgress(on_progress, out.size(), byte_source))
                    break;
            }
        }
        else
        {
            out.append(logical);
            archive_read_data_skip(handle.reader);
            if (!reportCatalogProgress(on_progress, out.size(), byte_source))
                break;
        }
    }
}

bool extractFromHandle(ArchiveHandle& handle,
                       const QString& entry_path,
                       const QString& dest_path,
                       const QString& container_label,
                       const QString& full_logical_path,
                       QString* error)
{
    const std::optional<logview_archive::NestedPathSplit> split = logview_archive::splitNestedEntryPath(entry_path);
    if (!split.has_value())
    {
        return locateAndExtractEntry(handle,
                                     entry_path,
                                     dest_path,
                                     container_label,
                                     full_logical_path,
                                     error);
    }

    QByteArray nested_bytes;
    if (!locateAndReadEntry(handle,
                            split->outer_entry,
                            nested_bytes,
                            container_label,
                            full_logical_path,
                            error))
    {
        return false;
    }

    ArchiveHandle nested;
    if (!openArchiveFromMemory(nested_bytes, nested, error))
    {
        if (error)
        {
            *error = Lang::tr("archive.error.extract_nested_open")
                         .arg(split->outer_entry, *error);
            *error += QLatin1Char('\n')
                      + Lang::tr("archive.error.extract_full_path").arg(full_logical_path);
        }
        return false;
    }

    return extractFromHandle(nested,
                            split->inner_path,
                            dest_path,
                            split->outer_entry,
                            full_logical_path,
                            error);
}

bool listFromPath(const QString& archive_path,
                  const QString& path_prefix,
                  int depth,
                  QStringList& out,
                  bool& truncated,
                  const ArchiveCatalog::ProgressCallback& on_progress)
{
    ArchiveHandle handle;
    if (!openArchiveFromPath(QFileInfo(archive_path).absoluteFilePath(), handle, nullptr))
        return false;

    listFromHandle(handle, path_prefix, depth, out, truncated, on_progress, handle.reader);
    return true;
}

}  // namespace

ArchiveCatalogResult ArchiveCatalog::build(const QString& archive_path,
                                           const ProgressCallback& on_progress)
{
    ArchiveCatalogResult result;
    listFromPath(QFileInfo(archive_path).absoluteFilePath(),
                 QString(),
                 0,
                 result.entry_paths,
                 result.truncated,
                 on_progress);
    result.entry_paths.sort(Qt::CaseInsensitive);
    return result;
}

bool ArchiveCatalog::extractNestedEntry(const QString& archive_path,
                                        const QString& entry_id,
                                        const QString& dest_path,
                                        QString* error)
{
    ArchiveHandle handle;
    const QString absolute = QFileInfo(archive_path).absoluteFilePath();
    if (!openArchiveFromPath(absolute, handle, error))
    {
        if (error)
        {
            *error = Lang::tr("archive.error.extract_open_root")
                         .arg(QFileInfo(absolute).fileName(), *error);
        }
        return false;
    }

    const QString normalized = logview_archive::normalizeEntryName(entry_id);
    return extractFromHandle(handle,
                             normalized,
                             dest_path,
                             QFileInfo(absolute).fileName(),
                             normalized,
                             error);
}
