#ifndef LINE_SOURCE_HPP
#define LINE_SOURCE_HPP

#include <cstdint>

#include <QtGlobal>

#include "Line.hpp"

class LineSource
{
public:
    virtual ~LineSource() = default;

    virtual qint64 count() const = 0;

    virtual Line at(qint64 index) const = 0;

    virtual uint32_t maxLineNumber() const = 0;

    virtual qint64 rowForLineNumber(uint32_t number) const;
};

#endif // LINE_SOURCE_HPP
