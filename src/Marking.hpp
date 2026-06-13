#pragma once

#include <QString>
#include <QIcon>

namespace serializer { class Marking; }

namespace marking_colors
{
struct Entry
{
    const char* name;
    const char* hex;
};

// Pastel colors that keep dark text readable when used as a background highlight.
inline const Entry kPalette[] = {
    {"Yellow", "#fff59d"},
    {"Green",  "#c5e1a5"},
    {"Cyan",   "#80deea"},
    {"Blue",   "#90caf9"},
    {"Purple", "#ce93d8"},
    {"Pink",   "#f48fb1"},
    {"Orange", "#ffcc80"},
    {"Lime",   "#e6ee9c"},
};

inline constexpr int kPaletteSize = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));

inline const char* kDefaultColor = kPalette[0].hex;

// Returns a color from the palette cycling by index (used for auto-assigning).
QString colorForIndex(int index);
}  // namespace marking_colors

// Builds a small filled square icon of the given color (for combo boxes / lists).
QIcon marking_color_icon(const QString& hex, int size = 16);

class Marking
{
public:
    Marking() = default;
    Marking(const QString& text, const QString& color);

    QString text_{};
    QString color_{};

    friend class serializer::Marking;
};
