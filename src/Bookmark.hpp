#ifndef BOOKMARK_HPP
#define BOOKMARK_HPP

#include <cstdint>

#include <QString>
#include <QPixmap>

struct Bookmark
{
    uint32_t line_number;
    QString bookmark_text;
    QPixmap icon;
};
#endif // BOOKMARK_HPP
