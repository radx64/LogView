#ifndef BOOKMARK_HPP
#define BOOKMARK_HPP

#include <cstdint>

#include <QString>
#include <QPixmap>

#include "Serializable.hpp"

class Bookmark : public Serializable
{
public:
    Bookmark() = default;
    Bookmark(const uint32_t &line_number, const QString &text, const QString &icon);
    virtual ~Bookmark() override = default ;

    uint32_t line_number_;
    QString text_;
    QString icon_;

    bool operator < (const Bookmark& b) const;

    virtual void serialize(QJsonObject &json) const override;
    virtual void deserialize(const QJsonObject &json) override;
};
#endif // BOOKMARK_HPP
