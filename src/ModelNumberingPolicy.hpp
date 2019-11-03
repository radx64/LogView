#ifndef MODEL_NUMBERING_POLICY_HPP
#define MODEL_NUMBERING_POLICY_HPP

#include <cstdint>

#include "ILineNumberingPolicy.hpp"
#include "Logfile.hpp"

class ModelNumberingPolicy : public ILineNumberingPolicy
{
public:
    ModelNumberingPolicy(const Lines& lines);
    uint32_t mapLineNumber(const uint32_t lineNumber) const override;

protected:
    const Lines& lines_{};
};
#endif // MODEL_NUMBERING_POLICY_HPP
