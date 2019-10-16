#include "Bookmark.hpp"

#include <QJsonObject>

Bookmark::Bookmark(const uint32_t &line_number, const QString &text, const QPixmap &icon)
{
    line_number_ = line_number;
    text_ = text;
    icon_ = icon;
}

bool Bookmark::operator < (const Bookmark& b) const
{
    return (line_number_ < b.line_number_);
}

void Bookmark::serialize(QJsonObject &json) const
{
    json["line"] = QString(line_number_);
    json["text"] = text_;
    json["icon"] = "TBD";
}
void Bookmark::deserialize(const QJsonObject &json)
{
    (void) json;
}
