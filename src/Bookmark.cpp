#include "Bookmark.hpp"

bool Bookmark::operator < (const Bookmark& b) const
{
    return (line_number < b.line_number);
}
