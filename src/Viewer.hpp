#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QWidget>

class TextRenderer;
class Viewer : public QWidget
{
public:
    Viewer(QWidget* parent);
    TextRenderer* text_;
};

#endif // VIEWER_HPP
