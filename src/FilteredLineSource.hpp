#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include <QString>
#include <QVector>

#include "LineSource.hpp"

class GrepNode;

class FilteredLineSource : public LineSource
{
public:

    FilteredLineSource(std::shared_ptr<LineSource> parent, const GrepNode* grep);

    using ProgressCallback = std::function<bool(qint64 done, qint64 total)>;
    bool compute(const ProgressCallback& on_progress = {});

    bool isLoaded() const { return loaded_.load(std::memory_order_acquire); }

    qint64 count() const override;
    Line at(qint64 index) const override;
    uint32_t lineNumberAt(qint64 index) const override;
    uint32_t maxLineNumber() const override;

private:
    std::shared_ptr<LineSource> parent_;
    QString pattern_;
    bool is_regex_ = false;
    bool is_case_sensitive_ = false;
    bool is_inverted_ = false;

    QVector<qint64> indices_;
    uint32_t max_line_number_ = 0;
    std::atomic<bool> loaded_{false};
};
