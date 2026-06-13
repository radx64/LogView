#ifndef LINE_HPP
#define LINE_HPP

#include <cstdint>

#include <QString>
#include <QVector>

struct Line
{
    uint32_t number;
    QString text;
};

using Lines = QVector<Line>;

#endif // LINE_HPP
