#include "Marking.hpp"

#include <QColor>
#include <QPixmap>

namespace marking_colors
{
QString colorForIndex(int index)
{
    if (kPaletteSize <= 0) return QString::fromLatin1("#ffff00");
    const int wrapped = ((index % kPaletteSize) + kPaletteSize) % kPaletteSize;
    return QString::fromLatin1(kPalette[wrapped].hex);
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
