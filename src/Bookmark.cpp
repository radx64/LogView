#include "Bookmark.hpp"

Bookmark::Bookmark(const uint32_t &line_number, const QString &text, const QString &icon)
{
    line_number_ = line_number;
    text_ = text;
    icon_ = icon;
}

bool Bookmark::operator < (const Bookmark& b) const
{
    return (line_number_ < b.line_number_);
}
