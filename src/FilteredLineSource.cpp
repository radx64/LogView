#include "FilteredLineSource.hpp"

#include <algorithm>

#include <QRegularExpression>
#include <QString>

#include "GrepNode.hpp"


FilteredLineSource::FilteredLineSource(std::shared_ptr<LineSource> parent, const GrepNode* grep)
    : parent_(std::move(parent))
    , pattern_(QString::fromStdString(grep->getPattern()))
    , is_regex_(grep->isRegEx())
    , is_case_sensitive_(grep->isCaseSensitive())
    , is_inverted_(grep->isInverted())
{
}

bool FilteredLineSource::compute(const ProgressCallback& on_progress)
{
    indices_.clear();
    max_line_number_ = 0;

    const qint64 total = parent_ ? parent_->count() : 0;


    const qint64 stride = std::clamp<qint64>(total / 100, 1000, 1 << 16);
    qint64 until_report = stride;

    const auto keep_going = [&](qint64 scanned) -> bool
    {
        if (--until_report > 0) return true;
        until_report = stride;
        return on_progress ? on_progress(scanned, total) : true;
    };

    if (is_regex_)
    {
        const QRegularExpression expression(pattern_);
        for (qint64 i = 0; i < total; ++i)
        {
            const Line line = parent_->at(i);
            const bool matched = expression.match(line.text).hasMatch();
            if (matched != is_inverted_)
            {
                indices_.append(i);
                max_line_number_ = std::max(max_line_number_, line.number);
            }
            if (!keep_going(i)) { indices_.clear(); return false; }
        }
    }
    else
    {
        const Qt::CaseSensitivity sensitivity =
            is_case_sensitive_ ? Qt::CaseSensitive : Qt::CaseInsensitive;
        for (qint64 i = 0; i < total; ++i)
        {
            const Line line = parent_->at(i);
            const bool matched = line.text.contains(pattern_, sensitivity);
            if (matched != is_inverted_)
            {
                indices_.append(i);
                max_line_number_ = std::max(max_line_number_, line.number);
            }
            if (!keep_going(i)) { indices_.clear(); return false; }
        }
    }

    loaded_.store(true, std::memory_order_release);
    if (on_progress) on_progress(total, total);
    return true;
}

qint64 FilteredLineSource::count() const
{
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    return indices_.size();
}

uint32_t FilteredLineSource::maxLineNumber() const
{
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    return max_line_number_;
}

Line FilteredLineSource::at(qint64 index) const
{
    if (!loaded_.load(std::memory_order_acquire)) return Line{0, QString()};
    if (index < 0 || index >= indices_.size()) return Line{0, QString()};
    return parent_->at(indices_.at(static_cast<int>(index)));
}

uint32_t FilteredLineSource::lineNumberAt(qint64 index) const
{
    if (!loaded_.load(std::memory_order_acquire)) return 0;
    if (index < 0 || index >= indices_.size()) return 0;
    return parent_->lineNumberAt(indices_.at(static_cast<int>(index)));
}