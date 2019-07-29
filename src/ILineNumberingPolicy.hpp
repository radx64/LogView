#ifndef ILINE_NUMBERING_POLICY_HPP
#define ILINE_NUMBERING_POLICY_HPP

#include <cstdint>

class ILineNumberingPolicy
{
public:
    virtual uint32_t mapLineNumber(const uint32_t lineNumber) const = 0;
    virtual ~ILineNumberingPolicy() = default;
};
#endif // ILINE_NUMBERING_POLICY_HPP
