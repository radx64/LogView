#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <memory>

#include <QPlainTextEdit>
#include "Logfile.hpp"

#include "ILineNumberingPolicy.hpp"

class TextRenderer : public QPlainTextEdit
{
public:
    TextRenderer(QWidget* parent,
        const Lines content,
        std::unique_ptr<ILineNumberingPolicy> lineRenderingPolicy);
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
    const Lines content_; //this need to be moved out of renderer;
    std::unique_ptr<ILineNumberingPolicy> lineRenderingPolicy_;
};

#endif  // TEXT_RENDERER_HPP
