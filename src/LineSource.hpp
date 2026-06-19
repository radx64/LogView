#pragma once

#include <cstdint>

#include <QtGlobal>

#include "Line.hpp"

class LineSource
{
public:
    virtual ~LineSource() = default;

    virtual qint64 count() const = 0;

    virtual Line at(qint64 index) const = 0;

    // Returns the original line number for a row without fetching its text,
    virtual uint32_t lineNumberAt(qint64 index) const;

    virtual uint32_t maxLineNumber() const = 0;

    virtual qint64 rowForLineNumber(uint32_t number) const;
};
