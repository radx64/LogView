#pragma once

#include <cstdint>

#include <QString>
#include <QVector>

struct Line
{
    uint32_t number;
    QString text;
};

using Lines = QVector<Line>;
