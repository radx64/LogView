#include "FileLineSource.hpp"

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
    if (!file_.open(QIODevice::ReadOnly))
    {
        QMessageBox msg;
        msg.setText("Unable to open file " + file_.errorString());
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
        return;
    }

    file_size_ = file_.size();
    buildIndex();
    valid_ = true;
}

void FileLineSource::buildIndex()
{
    line_offsets_.clear();
    if (file_size_ <= 0) return;

    line_offsets_.append(0);

    constexpr qint64 kReadBuffer = 1 << 20;
    QByteArray buffer;
    qint64 absolute = 0;

    file_.seek(0);
    while (!file_.atEnd())
    {
        buffer = file_.read(kReadBuffer);
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
    }
}

qint64 FileLineSource::count() const
{
    return line_offsets_.size();
}

uint32_t FileLineSource::maxLineNumber() const
{
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
    if (index < 0 || index >= line_offsets_.size()) return Line{0, QString()};

    const int chunk = static_cast<int>(index / kChunkLines);
    const int offset_in_chunk = static_cast<int>(index % kChunkLines);
    const QVector<QString>& lines = ensureChunk(chunk);

    const QString text = (offset_in_chunk < lines.size()) ? lines.at(offset_in_chunk) : QString();
    return Line{static_cast<uint32_t>(index + 1), text};
}
