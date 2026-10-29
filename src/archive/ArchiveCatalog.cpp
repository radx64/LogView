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

        if (entryPath(entry) != needle)
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

        if (entryPath(entry) != needle)
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
        return true;
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

void listFromHandle(ArchiveHandle& handle,
                    const QString& path_prefix,
                    int depth,
                    QStringList& out,
                    bool& truncated,
                    const ArchiveCatalog::ProgressCallback& on_progress)
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
            QByteArray nested_bytes;
            if (!readCurrentEntryToBuffer(handle, nested_bytes, nullptr))
                continue;

            ArchiveHandle nested;
            if (!openArchiveFromMemory(nested_bytes, nested, nullptr))
                continue;

            listFromHandle(nested,
                           logical + QLatin1Char('/'),
                           depth + 1,
                           out,
                           truncated,
                           on_progress);
        }
        else
        {
            out.append(logical);
            archive_read_data_skip(handle.reader);
            if (on_progress && !on_progress(out.size()))
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

    listFromHandle(handle, path_prefix, depth, out, truncated, on_progress);
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
