#include "LineNumberingBasedOnModelPolicy.hpp"

#include "Logfile.hpp"

LineNumberingBasedOnModelPolicy::LineNumberingBasedOnModelPolicy(const Lines& lines) : lines_(lines){};

uint32_t LineNumberingBasedOnModelPolicy::mapLineNumber(const uint32_t lineNumber) const
{
    return lines_.at(static_cast<int>(lineNumber)).number;
}
