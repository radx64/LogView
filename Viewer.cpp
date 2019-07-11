#include "Viewer.hpp"

#include <QTextEdit>

Viewer::Viewer(QWidget* parent) : parent_(parent)
{
    text_ = new QTextEdit();
    text_->setReadOnly(true);
    text_->setText("Viewer");
    text_->setFontFamily("Courier New");
    text_->setFontPointSize(10);
}
