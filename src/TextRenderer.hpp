#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <QPlainTextEdit>

#include "Logfile.hpp"

class TextRenderer : public QPlainTextEdit
{
public:
    TextRenderer(QWidget* parent, const Lines content);
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
    const Lines content_;
};

#endif  // TEXT_RENDERER_HPP
