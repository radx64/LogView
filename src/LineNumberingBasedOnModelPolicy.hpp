#ifndef LINE_NUMBERING_BASED_ON_MODEL_POLICY_HPP
#define LINE_NUMBERING_BASED_ON_MODEL_POLICY_HPP

#include <cstdint>

#include "ILineNumberingPolicy.hpp"
#include "Logfile.hpp"

class LineNumberingBasedOnModelPolicy : public ILineNumberingPolicy
{
public:
    LineNumberingBasedOnModelPolicy(const Lines& lines);
    uint32_t mapLineNumber(const uint32_t lineNumber) const override;

protected:
    const Lines& lines_{};
};
#endif // LINE_NUMBERING_BASED_ON_MODEL_POLICY_HPP
