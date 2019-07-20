#include "Viewer.hpp"
#include "TextRenderer.hpp"

Viewer::Viewer(QWidget* parent, const Lines& lines)
    :lines_(lines)
{
    setParent(parent);
    text_ = new TextRenderer(parent, lines_);
    text_->setReadOnly(true);
}
