#include "LineNumberArea.hpp"
#include "TextRenderer.hpp"

LineNumberArea::LineNumberArea(TextRenderer* renderer) : QWidget(renderer)
{
    textRenderer_ = renderer;
}

QSize LineNumberArea::sizeHint() const
{
    return {textRenderer_->lineNumberAreaWidth(), 0};
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    textRenderer_->lineNumberAreaPaintEvent(event);
}
