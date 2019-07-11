#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QWidget>

class QTextEdit;

/* This Viewer class is only a POC to be replaced later by something more usefull*/
class Viewer : public QWidget
{
public:
    Viewer(QWidget* parent);
    QTextEdit* text_;
};

#endif // VIEWER_HPP
