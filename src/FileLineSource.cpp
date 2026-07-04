#include "FileLineSource.hpp"

#include "Translator.hpp"

#include <QMessageBox>

namespace
{
constexpr int kTabWidth = 4;

QString expandTabs(const QString& in)
{
    if (!in.contains(QLatin1Char('\t'))) return in;

    QString out;
    out.reserve(in.size());
    int column = 0;
    for (const QChar ch : in)
    {
        if (ch == QLatin1Char('\t'))
        {
            const int spaces = kTabWidth - (column % kTabWidth);
            out.append(QString(spaces, QLatin1Char(' ')));
            column += spaces;
        }
        else
        {
            out.append(ch);
            ++column;
        }
    }
    return out;
}
}

FileLineSource::FileLineSource(const QString& filename)
    : filename_(filename), file_(filename)
{
    // Only open the read handle here (cheap). The expensive newline scan is
    // performed later in buildIndex(), which is meant to run on a worker
    // thread so the UI does not block while large files are loaded.
    if (!file_.open(QIODevice::ReadOnly))
    {
        QMessageBox msg;
        msg.setText(Lang::tr("file.open_failed").arg(file_.errorString()));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
        return;
    }

    file_size_ = file_.size();
    valid_ = true;
}

bool FileLineSource::buildIndex(const ProgressCallback& on_progress)
{
    line_offsets_.clear();

    if (!valid_ || file_size_ <= 0)
    {
        loaded_.store(true, std::memory_order_release);
        if (on_progress) on_progress(file_size_, file_size_);
        return true;
    }

    // Use a private read handle so the indexing thread never races with chunk
    // reads happening on the GUI thread through file_.
    QFile scan_file(filename_);
    if (!scan_file.open(QIODevice::ReadOnly))
    {
        loaded_.store(true, std::memory_order_release);
        if (on_progress) on_progress(file_size_, file_size_);
        return true;
    }

    line_offsets_.append(0);

    constexpr qint64 kReadBuffer = 1 << 20;
    QByteArray buffer;
    qint64 absolute = 0;

    while (!scan_file.atEnd())
    {
        buffer = scan_file.read(kReadBuffer);
        const int size = buffer.size();
        const char* data = buffer.constData();
        for (int i = 0; i < size; ++i)
        {
            if (data[i] == '\n')
            {
                const qint64 next = absolute + i + 1;
                if (next < file_size_) line_offsets_.append(next);
            }
        }
        absolute += size;

        if (on_progress && !on_progress(absolute, file_size_))
        {
            line_offsets_.clear();
            return false;
        }
    }

    loaded_.store(true, std::memory_order_release);
    if (on_progress) on_progress(file_size_, file_size_);
    return true;
}

qint64 FileLineSource::count() const
{
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    return line_offsets_.size();
}

uint32_t FileLineSource::lineNumberAt(qint64 index) const
{
    // The file line number is just the row index + 1; no chunk read required.
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    if (index < 0 || index >= line_offsets_.size()) return 0;
    return static_cast<uint32_t>(index + 1);
}

uint32_t FileLineSource::maxLineNumber() const
{
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    return static_cast<uint32_t>(line_offsets_.size());
}

qint64 FileLineSource::rowForLineNumber(uint32_t number) const
{
    const qint64 total = count();
    if (total <= 0) return 0;
    qint64 row = static_cast<qint64>(number) - 1;
    if (row < 0) row = 0;
    if (row >= total) row = total - 1;
    return row;
}

const QVector<QString>& FileLineSource::ensureChunk(int chunk) const
{
    auto it = cache_.find(chunk);
    if (it != cache_.end())
    {
        lru_.removeOne(chunk);
        lru_.prepend(chunk);
        return it.value();
    }

    const qint64 total = line_offsets_.size();
    const qint64 first_line = static_cast<qint64>(chunk) * kChunkLines;
    const qint64 last_line = qMin(first_line + kChunkLines, total);
    const int expected = static_cast<int>(last_line - first_line);

    QVector<QString> lines;
    lines.reserve(expected);

    if (expected > 0)
    {
        const qint64 start_offset = line_offsets_.at(static_cast<int>(first_line));
        const qint64 end_offset = (last_line < total)
            ? line_offsets_.at(static_cast<int>(last_line))
            : file_size_;

        file_.seek(start_offset);
        const QByteArray buffer = file_.read(end_offset - start_offset);
        const int size = buffer.size();
        const char* data = buffer.constData();

        int line_start = 0;
        for (int i = 0; i < size && lines.size() < expected; ++i)
        {
            if (data[i] == '\n')
            {
                lines.append(expandTabs(QString::fromUtf8(data + line_start, i - line_start).trimmed()));
                line_start = i + 1;
            }
        }
        if (lines.size() < expected)
        {
            lines.append(expandTabs(QString::fromUtf8(data + line_start, size - line_start).trimmed()));
        }
    }

    cache_.insert(chunk, std::move(lines));
    lru_.prepend(chunk);

    while (lru_.size() > kMaxCachedChunks)
    {
        const int victim = lru_.takeLast();
        cache_.remove(victim);
    }

    return cache_[chunk];
}

Line FileLineSource::at(qint64 index) const
{
    if (!loaded_.load(std::memory_order_acquire)) return Line{0, QString()};
    if (index < 0 || index >= line_offsets_.size()) return Line{0, QString()};

    const int chunk = static_cast<int>(index / kChunkLines);
    const int offset_in_chunk = static_cast<int>(index % kChunkLines);

    // Hold the lock across the cache lookup and the copy out of the cached
    // chunk: another thread may evict/replace the chunk we are referencing.
    QMutexLocker lock(&read_mutex_);
    const QVector<QString>& lines = ensureChunk(chunk);
    const QString text = (offset_in_chunk < lines.size()) ? lines.at(offset_in_chunk) : QString();
    return Line{static_cast<uint32_t>(index + 1), text};
}
