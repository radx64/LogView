#include "FilteredLineSource.hpp"

#include <algorithm>

#include <QRegularExpression>
#include <QString>

#include "GrepNode.hpp"

FilteredLineSource::FilteredLineSource(std::shared_ptr<LineSource> parent,
                                       QVector<qint64> indices,
                                       uint32_t max_line_number)
    : parent_(std::move(parent))
    , indices_(std::move(indices))
    , max_line_number_(max_line_number)
{
}

std::shared_ptr<FilteredLineSource> FilteredLineSource::create(
    std::shared_ptr<LineSource> parent, const GrepNode* grep)
{
    QVector<qint64> indices;
    uint32_t max_number = 0;

    const QString pattern = QString::fromStdString(grep->getPattern());
    const bool inverted = grep->isInverted();
    const qint64 total = parent->count();

    if (grep->isRegEx())
    {
        const QRegularExpression expression(pattern);
        for (qint64 i = 0; i < total; ++i)
        {
            const Line line = parent->at(i);
            const bool matched = expression.match(line.text).hasMatch();
            if (matched != inverted)
            {
                indices.append(i);
                max_number = std::max(max_number, line.number);
            }
        }
    }
    else
    {
        const Qt::CaseSensitivity sensitivity =
            grep->isCaseInsensitive() ? Qt::CaseInsensitive : Qt::CaseSensitive;
        for (qint64 i = 0; i < total; ++i)
        {
            const Line line = parent->at(i);
            const bool matched = line.text.contains(pattern, sensitivity);
            if (matched != inverted)
            {
                indices.append(i);
                max_number = std::max(max_number, line.number);
            }
        }
    }

    return std::make_shared<FilteredLineSource>(std::move(parent), std::move(indices), max_number);
}

qint64 FilteredLineSource::count() const
{
    return indices_.size();
}

uint32_t FilteredLineSource::maxLineNumber() const
{
    return max_line_number_;
}

Line FilteredLineSource::at(qint64 index) const
{
    if (index < 0 || index >= indices_.size()) return Line{0, QString()};
    return parent_->at(indices_.at(static_cast<int>(index)));
}
