#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QWidget>
#include "Logfile.hpp"

class TextRenderer;

class Viewer : public QWidget
{
public:
    Viewer(QWidget* parent, const Lines& lines);
    TextRenderer* text_;
    const Lines lines_;
};

#endif // VIEWER_HPP
