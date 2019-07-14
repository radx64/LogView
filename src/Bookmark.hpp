#ifndef BOOKMARK_HPP
#define BOOKMARK_HPP

#include <cstdint>
#include <QString>

struct Bookmark
{
    uint32_t line_number;
    QString bookmark_text;
};
#endif // BOOKMARK_HPP
