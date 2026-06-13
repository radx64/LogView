#include "Marking.hpp"

#include <QColor>
#include <QPixmap>
#include <QRandomGenerator>

namespace marking_colors
{
QString colorForIndex(int index)
{
    if (kPaletteSize <= 0) return QString::fromLatin1("#ffff00");
    const int wrapped = ((index % kPaletteSize) + kPaletteSize) % kPaletteSize;
    return QString::fromLatin1(kPalette[wrapped].hex);
}

QString randomColor()
{
    if (kPaletteSize <= 0) return QString::fromLatin1("#ffff00");
    const int index = QRandomGenerator::global()->bounded(kPaletteSize);
    return QString::fromLatin1(kPalette[index].hex);
}

int paletteIndexOf(const QString& hex)
{
    const QColor target(hex);
    for (int i = 0; i < kPaletteSize; ++i)
    {
        if (QColor(QString::fromLatin1(kPalette[i].hex)) == target)
            return i;
    }
    return -1;
}

QString nextColor(const QString& hex)
{
    if (kPaletteSize <= 0) return QString::fromLatin1("#ffff00");
    const int current = paletteIndexOf(hex);
    const int next = (current + 1) % kPaletteSize;
    return QString::fromLatin1(kPalette[next].hex);
}
}  // namespace marking_colors

QIcon marking_color_icon(const QString& hex, int size)
{
    QPixmap pixmap(size, size);
    QColor color(hex);
    if (!color.isValid()) color = QColor(QString::fromLatin1(marking_colors::kDefaultColor));
    pixmap.fill(color);
    return QIcon(pixmap);
}

Marking::Marking(const QString& text, const QString& color)
    : text_(text), color_(color)
{
}
