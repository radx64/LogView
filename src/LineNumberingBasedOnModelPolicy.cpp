#include "LineNumberingBasedOnModelPolicy.hpp"

#include <QDebug>

#include "Logfile.hpp"

LineNumberingBasedOnModelPolicy::LineNumberingBasedOnModelPolicy(const Lines& lines) : lines_(lines){};

uint32_t LineNumberingBasedOnModelPolicy::mapLineNumber(const uint32_t lineNumber) const
{
    if (lineNumber < static_cast<uint32_t>(lines_.size())) return lines_.at(static_cast<int>(lineNumber)).number;
    else
    {
        qDebug() << "Trying to get lines out of scope!";
        return UINT32_MAX;
    }
}
