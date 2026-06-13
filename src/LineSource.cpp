#include "LineSource.hpp"

qint64 LineSource::rowForLineNumber(uint32_t number) const
{
    const qint64 total = count();
    if (total <= 0) return 0;

    qint64 lo = 0;
    qint64 hi = total;
    while (lo < hi)
    {
        const qint64 mid = lo + (hi - lo) / 2;
        if (at(mid).number < number) lo = mid + 1;
        else hi = mid;
    }
    if (lo >= total) lo = total - 1;
    return lo;
}
