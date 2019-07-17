#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <QPlainTextEdit>

class TextRenderer : public QPlainTextEdit
{
public:
    TextRenderer(QWidget* parent);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

#endif  // TEXT_RENDERER_HPP
