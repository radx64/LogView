#pragma once

#include <cstdint>

#include <QAbstractScrollArea>
#include <QFont>

class LineSource;
class MarkingsModel;

class ChunkedTextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    ChunkedTextView(QWidget* parent, LineSource* source, MarkingsModel* markings = nullptr);

    qint64 currentLine() const { return caret_line_; }

    void gotoRow(qint64 row);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void changeEvent(QEvent* event) override;

private:
    struct Pos
    {
        qint64 line = 0;
        int col = 0;
        bool operator==(const Pos& o) const { return line == o.line && col == o.col; }
        bool operator!=(const Pos& o) const { return !(*this == o); }
        bool operator<(const Pos& o) const { return line < o.line || (line == o.line && col < o.col); }
    };

    int gutterWidth() const;
    int visibleLineCount() const;
    qint64 topLine() const;
    int horizontalOffset() const;
    void updateScrollBars();
    void applySettings();
    Pos posAt(const QPoint& viewportPos) const;
    void ensureVisible(qint64 line);
    void moveCaret(qint64 line, int col, bool extendSelection);
    bool hasSelection() const { return sel_anchor_ != sel_caret_; }
    QString selectedText() const;
    void copySelection() const;
    void markSelection();

    LineSource* source_;
    MarkingsModel* markings_ = nullptr;
    QFont font_;
    int line_height_ = 1;
    int char_width_ = 1;
    qreal char_width_f_ = 1.0;
    int ascent_ = 0;

    qint64 caret_line_ = 0;
    int caret_col_ = 0;
    Pos sel_anchor_;
    Pos sel_caret_;
    bool selecting_ = false;

    mutable int max_line_width_chars_ = 0;
};
