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

QString colorForIndex(int index);

QString randomColor();

int paletteIndexOf(const QString& hex);

QString nextColor(const QString& hex);
}  // namespace marking_colors

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
