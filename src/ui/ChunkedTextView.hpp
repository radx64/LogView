#pragma once

#include <cstdint>
#include <memory>

#include <QAbstractScrollArea>
#include <QFont>
#include <QVector>

class LineSource;
class MarkingsModel;
class SearchWidget;
class BackgroundTask;
class QTimer;

class ChunkedTextView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    ChunkedTextView(QWidget* parent, std::shared_ptr<LineSource> source, MarkingsModel* markings = nullptr);

    qint64 currentLine() const { return caret_line_; }

    void gotoRow(qint64 row);

    // Recomputes scrollbars and repaints. Call after the backing LineSource
    // has finished loading so the freshly-available content becomes visible.
    void refresh();

    void showSearch();
    void hideSearch();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
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

    struct Match
    {
        qint64 line = 0;
        int col = 0;
        int len = 0;
    };

    int gutterWidth() const;
    int visibleLineCount() const;
    qint64 topLine() const;
    int horizontalOffset() const;
    void updateScrollBars();
    void applySettings();
    void adjustFontSize(int delta);
    Pos posAt(const QPoint& viewportPos) const;
    void ensureVisible(qint64 line);
    void moveCaret(qint64 line, int col, bool extendSelection);
    bool hasSelection() const { return sel_anchor_ != sel_caret_; }
    QString selectedText() const;
    void copySelection() const;
    void markSelection();

    void positionSearchWidget();
    void onSearchQueryChanged();
    void runSearch();
    void cancelSearch();
    void flushPendingSearch();
    void findNext();
    void findPrevious();
    void gotoMatch(int index);
    void updateSearchInfo();

    LineSource* source_;
    std::weak_ptr<LineSource> source_ref_;
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

    enum class PendingNav { None, Next, Previous };

    SearchWidget* search_widget_ = nullptr;
    QTimer* search_debounce_ = nullptr;
    BackgroundTask* search_task_ = nullptr;
    PendingNav pending_nav_ = PendingNav::None;
    QString search_pattern_;
    bool search_regex_ = false;
    bool search_case_ = false;
    bool search_active_ = false;
    QVector<Match> matches_;
    int current_match_ = -1;
};
