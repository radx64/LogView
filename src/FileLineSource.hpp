#pragma once

#include <cstdint>

#include <QFile>
#include <QHash>
#include <QList>
#include <QString>
#include <QVector>

#include "LineSource.hpp"

class FileLineSource : public LineSource
{
public:
    explicit FileLineSource(const QString& filename);

    qint64 count() const override;
    Line at(qint64 index) const override;
    uint32_t maxLineNumber() const override;
    qint64 rowForLineNumber(uint32_t number) const override;

    bool isValid() const { return valid_; }

private:
    static constexpr int kChunkLines = 4096;
    static constexpr int kMaxCachedChunks = 8;

    void buildIndex();
    const QVector<QString>& ensureChunk(int chunk) const;

    QString filename_;
    mutable QFile file_;
    QVector<qint64> line_offsets_;
    qint64 file_size_ = 0;
    bool valid_ = false;

    mutable QHash<int, QVector<QString>> cache_;
    mutable QList<int> lru_;
};
