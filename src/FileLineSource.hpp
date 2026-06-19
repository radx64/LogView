#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

#include <QFile>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>
#include <QVector>

#include "LineSource.hpp"

class FileLineSource : public LineSource
{
public:
    explicit FileLineSource(const QString& filename);

    qint64 count() const override;
    Line at(qint64 index) const override;
    uint32_t lineNumberAt(qint64 index) const override;
    uint32_t maxLineNumber() const override;
    qint64 rowForLineNumber(uint32_t number) const override;

    bool isValid() const { return valid_; }
    bool isLoaded() const { return loaded_.load(std::memory_order_acquire); }
    qint64 fileSize() const { return file_size_; }

    using ProgressCallback = std::function<bool(qint64 done, qint64 total)>;
    bool buildIndex(const ProgressCallback& on_progress = {});

private:
    static constexpr int kChunkLines = 256;
    static constexpr int kMaxCachedChunks = 64;

    const QVector<QString>& ensureChunk(int chunk) const;

    QString filename_;
    mutable QFile file_;
    QVector<qint64> line_offsets_;
    qint64 file_size_ = 0;
    bool valid_ = false;
    std::atomic<bool> loaded_{false};

    mutable QMutex read_mutex_;
    mutable QHash<int, QVector<QString>> cache_;
    mutable QList<int> lru_;
};
