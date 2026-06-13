#ifndef FILTERED_LINE_SOURCE_HPP
#define FILTERED_LINE_SOURCE_HPP

#include <cstdint>
#include <memory>

#include <QVector>

#include "LineSource.hpp"

class GrepNode;

class FilteredLineSource : public LineSource
{
public:
    FilteredLineSource(std::shared_ptr<LineSource> parent,
                       QVector<qint64> indices,
                       uint32_t max_line_number);

    static std::shared_ptr<FilteredLineSource> create(
        std::shared_ptr<LineSource> parent, const GrepNode* grep);

    qint64 count() const override;
    Line at(qint64 index) const override;
    uint32_t maxLineNumber() const override;

private:
    std::shared_ptr<LineSource> parent_;
    QVector<qint64> indices_;
    uint32_t max_line_number_ = 0;
};

#endif // FILTERED_LINE_SOURCE_HPP
