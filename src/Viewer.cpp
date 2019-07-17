#include "Viewer.hpp"
#include "TextRenderer.hpp"

Viewer::Viewer(QWidget* parent)
{
    setParent(parent);
    text_ = new TextRenderer(parent);
    text_->setReadOnly(true);
}
