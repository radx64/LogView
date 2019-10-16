#ifndef BOOKMARK_HPP
#define BOOKMARK_HPP

#include <cstdint>

#include <QString>
#include <QPixmap>

namespace serializer { class Bookmark; }

class Bookmark
{
public:
    Bookmark() = default;
    Bookmark(const uint32_t &line_number, const QString &text, const QString &icon);

    uint32_t line_number_;
    QString text_;
    QString icon_;

    bool operator < (const Bookmark& b) const;

    friend class serializer::Bookmark;
};
#endif // BOOKMARK_HPP
