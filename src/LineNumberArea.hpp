#ifndef LINE_NUMBER_AREA_HPP
#define LINE_NUMBER_AREA_HPP

#include <QWidget>

class TextRenderer;

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextRenderer* renderer);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    TextRenderer* textRenderer_;
};

#endif  // LINE_NUMBER_AREA_HPP
