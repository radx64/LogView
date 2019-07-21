#include "TextRenderer.hpp"

#include <QDebug>
#include <QColor>
#include <QList>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>

#include "LineNumberArea.hpp"

QString linesToQString(const Lines& lines)
{
    QString result;
    for (const auto& line : lines)
    {
        result.append(line.text);
        qDebug() << line.text;
    }
    return result;
}

TextRenderer::TextRenderer(QWidget* parent,
    const Lines content,
    std::unique_ptr<ILineNumberingPolicy> lineRenderingPolicy)
    : QPlainTextEdit(parent), content_(content), lineRenderingPolicy_(std::move(lineRenderingPolicy))
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &TextRenderer::blockCountChanged, this, &TextRenderer::updateLineNumberAreaWidth);
    connect(this, &TextRenderer::updateRequest, this, &TextRenderer::updateLineNumberArea);
    connect(this, &TextRenderer::cursorPositionChanged, this, &TextRenderer::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setFont(QFont(QString("Courier New")));
    setReadOnly(true);
    setPlainText(linesToQString(content));
}

void TextRenderer::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

int TextRenderer::lineNumberAreaWidth()
{
    int digits = 1;
    uint32_t max = qMax(1u, content_.last().number);
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void TextRenderer::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextRenderer::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) lineNumberArea->scroll(0, dy);
    else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void TextRenderer::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextRenderer::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(),  QColor(Qt::gray).lighter(150));
    QTextBlock block = firstVisibleBlock();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(lineRenderingPolicy_->mapLineNumber(
                static_cast<uint32_t>(block.blockNumber())));
            painter.setFont(QFont(QString("Courier New")));
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
    }
}
