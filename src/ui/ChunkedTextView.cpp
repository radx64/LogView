#include "ChunkedTextView.hpp"

#include <algorithm>

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>
#include <QWheelEvent>
#include <QtMath>

#include "AutoMarkingsModel.hpp"
#include "LineSource.hpp"
#include "MarkingsModel.hpp"
#include "SearchWidget.hpp"
#include "Settings.hpp"

namespace
{
constexpr int kGutterPadding = 8;
constexpr int kTextLeftPadding = 4;
constexpr int kMinFontPointSize = 6;
constexpr int kMaxFontPointSize = 72;
constexpr int kSearchDebounceMs = 120;
const QColor kDefaultSearchHighlightColor(255, 215, 0, 120);
const QColor kDefaultSearchCurrentMatchColor(255, 140, 0, 200);
}

ChunkedTextView::ChunkedTextView(QWidget* parent, LineSource* source, MarkingsModel* markings)
    : QAbstractScrollArea(parent), source_(source), markings_(markings)
{
    if (markings_)
    {
        connect(markings_, &MarkingsModel::changed,
                viewport(), QOverload<>::of(&QWidget::update));
    }
    connect(&AutoMarkingsModel::instance(), &AutoMarkingsModel::changed,
            viewport(), QOverload<>::of(&QWidget::update));

    connect(&Settings::instance(), &Settings::changed,
            this, &ChunkedTextView::applySettings);

    setFocusPolicy(Qt::StrongFocus);
    viewport()->setCursor(Qt::IBeamCursor);
    viewport()->setBackgroundRole(QPalette::Base);
    setFrameStyle(QFrame::NoFrame);

    verticalScrollBar()->setSingleStep(1);

    applySettings();
}

void ChunkedTextView::applySettings()
{
    font_ = Settings::instance().editorFont();
    font_.setStyleHint(QFont::Monospace);

    const QFontMetrics metrics(font_);
    line_height_ = metrics.height();
    ascent_ = metrics.ascent();

    const QFontMetricsF metricsF(font_);
    char_width_f_ = std::max<qreal>(1.0, metricsF.horizontalAdvance(QLatin1Char('9')));
    char_width_ = std::max(1, qRound(char_width_f_));

    horizontalScrollBar()->setSingleStep(char_width_);

    updateScrollBars();
    viewport()->update();
}

int ChunkedTextView::gutterWidth() const
{
    int digits = 1;
    uint32_t max = source_ ? source_->maxLineNumber() : 0;
    while (max >= 10) { max /= 10; ++digits; }
    return kGutterPadding * 2 + qCeil(char_width_f_ * digits);
}

int ChunkedTextView::visibleLineCount() const
{
    return std::max(1, viewport()->height() / line_height_);
}

qint64 ChunkedTextView::topLine() const
{
    return verticalScrollBar()->value();
}

int ChunkedTextView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

void ChunkedTextView::updateScrollBars()
{
    const qint64 total = source_ ? source_->count() : 0;
    const int visible = visibleLineCount();

    verticalScrollBar()->setRange(0, static_cast<int>(std::max<qint64>(0, total - visible)));
    verticalScrollBar()->setPageStep(visible);

    const int text_area = std::max(0, viewport()->width() - gutterWidth() - kTextLeftPadding);
    const int content_width = qCeil(max_line_width_chars_ * char_width_f_);
    horizontalScrollBar()->setRange(0, std::max(0, content_width - text_area));
    horizontalScrollBar()->setPageStep(text_area);
}

void ChunkedTextView::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBars();
    if (search_widget_ && search_widget_->isVisible())
        positionSearchWidget();
}

void ChunkedTextView::wheelEvent(QWheelEvent* event)
{
    if (!event->modifiers().testFlag(Qt::ControlModifier))
    {
        QAbstractScrollArea::wheelEvent(event);
        return;
    }

    const int steps = event->angleDelta().y();
    if (steps == 0)
    {
        event->accept();
        return;
    }

    QFont font = Settings::instance().editorFont();
    const int current_size = font.pointSize() > 0 ? font.pointSize() : 10;
    const int new_size = std::clamp(current_size + (steps > 0 ? 1 : -1),
                                    kMinFontPointSize, kMaxFontPointSize);

    event->accept();
    if (new_size == current_size)
        return;

    font.setPointSize(new_size);
    Settings::instance().setEditorFont(font);
}

void ChunkedTextView::scrollContentsBy(int /*dx*/, int /*dy*/)
{
    viewport()->update();
}

void ChunkedTextView::changeEvent(QEvent* event)
{
    QAbstractScrollArea::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::ApplicationPaletteChange ||
        event->type() == QEvent::StyleChange)
    {
        viewport()->update();
    }
}

ChunkedTextView::Pos ChunkedTextView::posAt(const QPoint& viewportPos) const
{
    const qint64 total = source_ ? source_->count() : 0;
    Pos pos;
    if (total <= 0) return pos;

    qint64 line = topLine() + viewportPos.y() / line_height_;
    line = std::clamp<qint64>(line, 0, total - 1);

    const qreal text_x = viewportPos.x() - gutterWidth() - kTextLeftPadding + horizontalOffset();
    int col = (text_x > 0) ? static_cast<int>((text_x + char_width_f_ / 2.0) / char_width_f_) : 0;
    const int len = static_cast<int>(source_->at(line).text.length());
    col = std::clamp(col, 0, len);

    pos.line = line;
    pos.col = col;
    return pos;
}

void ChunkedTextView::ensureVisible(qint64 line)
{
    const qint64 top = topLine();
    const int visible = visibleLineCount();
    if (line < top)
    {
        verticalScrollBar()->setValue(static_cast<int>(line));
    }
    else if (line >= top + visible)
    {
        verticalScrollBar()->setValue(static_cast<int>(line - visible + 1));
    }
}

void ChunkedTextView::moveCaret(qint64 line, int col, bool extendSelection)
{
    const qint64 total = source_ ? source_->count() : 0;
    if (total <= 0) return;

    caret_line_ = std::clamp<qint64>(line, 0, total - 1);
    const int len = static_cast<int>(source_->at(caret_line_).text.length());
    caret_col_ = std::clamp(col, 0, len);

    sel_caret_ = Pos{caret_line_, caret_col_};
    if (!extendSelection) sel_anchor_ = sel_caret_;

    ensureVisible(caret_line_);
    viewport()->update();
}

void ChunkedTextView::gotoRow(qint64 row)
{
    const qint64 total = source_ ? source_->count() : 0;
    if (total <= 0) return;

    row = std::clamp<qint64>(row, 0, total - 1);
    caret_line_ = row;
    caret_col_ = 0;
    sel_anchor_ = sel_caret_ = Pos{row, 0};

    const int visible = visibleLineCount();
    const qint64 target = std::max<qint64>(0, row - visible / 2);
    verticalScrollBar()->setValue(static_cast<int>(target));
    viewport()->update();
}

void ChunkedTextView::paintEvent(QPaintEvent* event)
{
    QPainter painter(viewport());
    painter.setFont(font_);

    const QPalette pal = palette();
    const QColor background_color = pal.color(QPalette::Base);
    const QColor text_color = pal.color(QPalette::Text);
    const QColor gutter_background_color = pal.color(QPalette::Window);
    const QColor gutter_text_color = pal.color(QPalette::WindowText);

    painter.fillRect(event->rect(), background_color);

    const qint64 total = source_ ? source_->count() : 0;
    const int gutter = gutterWidth();
    const int text_x0 = gutter + kTextLeftPadding - horizontalOffset();

    if (total > 0)
    {
        const qint64 first = topLine();
        const qint64 last = std::min<qint64>(total, first + visibleLineCount() + 1);

        QColor current_line_color = Settings::instance().highlightLineColor();
        if (!current_line_color.isValid())
        {
            current_line_color = pal.color(QPalette::Highlight);
            current_line_color.setAlpha(45);
        }
        QColor selection_color = pal.color(QPalette::Highlight);
        selection_color.setAlpha(110);

        QColor search_all_color = Settings::instance().searchHighlightColor();
        if (!search_all_color.isValid())
            search_all_color = kDefaultSearchHighlightColor;
        QColor search_current_color = Settings::instance().searchCurrentMatchColor();
        if (!search_current_color.isValid())
            search_current_color = kDefaultSearchCurrentMatchColor;
        const bool show_search = search_active_ && !matches_.isEmpty();
        int search_cursor = 0;

        Pos sel_start = sel_anchor_;
        Pos sel_end = sel_caret_;
        if (sel_end < sel_start) std::swap(sel_start, sel_end);
        const bool selection = hasSelection();

        painter.save();
        painter.setClipRect(gutter, 0, viewport()->width() - gutter, viewport()->height());
        for (qint64 row = first; row < last; ++row)
        {
            const int y = static_cast<int>((row - first) * line_height_);
            const QString text = source_->at(row).text;
            max_line_width_chars_ = std::max(max_line_width_chars_, static_cast<int>(text.length()));

            if (row == caret_line_)
            {
                painter.fillRect(0, y, viewport()->width(), line_height_, current_line_color);
            }

            struct MarkedSegment { QRect rect; QColor textColor; };
            QVector<MarkedSegment> marked_segments;
            QRegion marked_region;

            if (markings_)
            {
                for (const Marking& marking : markings_->markings())
                {
                    if (marking.text_.isEmpty()) continue;
                    QColor marking_color(marking.color_);
                    if (!marking_color.isValid()) continue;

                    const QColor marking_text_color(marking.effectiveTextColor());

                    const int needle_len = static_cast<int>(marking.text_.length());
                    int from = 0;
                    while (true)
                    {
                        const int idx = text.indexOf(marking.text_, from, Qt::CaseSensitive);
                        if (idx < 0) break;
                        const int sx = qRound(text_x0 + idx * char_width_f_);
                        const int ex = qRound(text_x0 + (idx + needle_len) * char_width_f_);
                        const QRect rect(sx, y, ex - sx, line_height_);
                        painter.fillRect(rect, marking_color);
                        marked_segments.append({rect, marking_text_color});
                        marked_region += rect;
                        from = idx + needle_len;
                    }
                }
            }

            for (const AutoMarking& marking : AutoMarkingsModel::instance().markings())
            {
                if (!marking.enabled_ || marking.pattern_.isEmpty()) continue;

                const QColor marking_text_color(marking.text_color_);
                if (!marking_text_color.isValid()) continue;

                const QRegularExpression expression = marking.regularExpression();
                if (!expression.isValid()) continue;

                const QColor background_color(marking.background_color_);
                QRegularExpressionMatchIterator matches = expression.globalMatch(text);
                while (matches.hasNext())
                {
                    const QRegularExpressionMatch match = matches.next();
                    const int idx = match.capturedStart();
                    const int len = match.capturedLength();
                    if (idx < 0 || len <= 0) continue;

                    const int sx = qRound(text_x0 + idx * char_width_f_);
                    const int ex = qRound(text_x0 + (idx + len) * char_width_f_);
                    const QRect rect(sx, y, ex - sx, line_height_);
                    if (background_color.isValid())
                        painter.fillRect(rect, background_color);
                    marked_segments.append({rect, marking_text_color});
                    marked_region += rect;
                }
            }

            if (show_search)
            {
                while (search_cursor < matches_.size() && matches_[search_cursor].line < row)
                    ++search_cursor;
                for (int i = search_cursor; i < matches_.size() && matches_[i].line == row; ++i)
                {
                    const Match& match = matches_[i];
                    const int sx = qRound(text_x0 + match.col * char_width_f_);
                    const int ex = qRound(text_x0 + (match.col + match.len) * char_width_f_);
                    painter.fillRect(sx, y, ex - sx, line_height_,
                        (i == current_match_) ? search_current_color : search_all_color);
                }
            }

            if (selection && row >= sel_start.line && row <= sel_end.line)
            {
                const int len = static_cast<int>(text.length());
                const int from = (row == sel_start.line) ? sel_start.col : 0;
                int to = (row == sel_end.line) ? sel_end.col : len;
                if (row != sel_end.line) to = len + 1;
                const int sx = qRound(text_x0 + from * char_width_f_);
                const int ex = qRound(text_x0 + to * char_width_f_);
                painter.fillRect(sx, y, ex - sx, line_height_, selection_color);
            }

            if (marked_segments.isEmpty())
            {
                painter.setPen(text_color);
                painter.drawText(text_x0, y + ascent_, text);
            }
            else
            {
                painter.save();
                painter.setClipRegion(
                    QRegion(0, y, viewport()->width(), line_height_) - marked_region,
                    Qt::IntersectClip);
                painter.setPen(text_color);
                painter.drawText(text_x0, y + ascent_, text);
                painter.restore();

                for (const MarkedSegment& segment : marked_segments)
                {
                    painter.save();
                    painter.setClipRect(segment.rect, Qt::IntersectClip);
                    painter.setPen(segment.textColor);
                    painter.drawText(text_x0, y + ascent_, text);
                    painter.restore();
                }
            }
        }
        painter.restore();

        painter.fillRect(0, 0, gutter, viewport()->height(), gutter_background_color);
        painter.setPen(gutter_text_color);
        for (qint64 row = first; row < last; ++row)
        {
            const int y = static_cast<int>((row - first) * line_height_);
            const QString number = QString::number(source_->at(row).number);
            painter.drawText(0, y, gutter - kGutterPadding, line_height_,
                Qt::AlignRight | Qt::AlignVCenter, number);
        }
    }
    else
    {
        painter.fillRect(0, 0, gutter, viewport()->height(), gutter_background_color);
    }

    const int text_area = std::max(0, viewport()->width() - gutter - kTextLeftPadding);
    const int content_width = qCeil(max_line_width_chars_ * char_width_f_);
    if (horizontalScrollBar()->maximum() != std::max(0, content_width - text_area))
    {
        updateScrollBars();
    }
}

void ChunkedTextView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    setFocus();
    const Pos pos = posAt(event->position().toPoint());
    sel_anchor_ = sel_caret_ = pos;
    caret_line_ = pos.line;
    caret_col_ = pos.col;
    selecting_ = true;
    viewport()->update();
}

void ChunkedTextView::mouseMoveEvent(QMouseEvent* event)
{
    if (!selecting_) return;
    const QPoint p = event->position().toPoint();

    if (p.y() < 0) verticalScrollBar()->setValue(verticalScrollBar()->value() - 1);
    else if (p.y() > viewport()->height()) verticalScrollBar()->setValue(verticalScrollBar()->value() + 1);

    const Pos pos = posAt(p);
    sel_caret_ = pos;
    caret_line_ = pos.line;
    caret_col_ = pos.col;
    viewport()->update();
}

void ChunkedTextView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) selecting_ = false;
}

void ChunkedTextView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !source_ || source_->count() <= 0)
    {
        QAbstractScrollArea::mouseDoubleClickEvent(event);
        return;
    }

    setFocus();
    const Pos pos = posAt(event->position().toPoint());
    const QString text = source_->at(pos.line).text;
    const int len = static_cast<int>(text.length());

    if (len == 0)
    {
        sel_anchor_ = sel_caret_ = pos;
        caret_line_ = pos.line;
        caret_col_ = 0;
        viewport()->update();
        return;
    }

    const auto isWordChar = [](QChar ch) { return ch.isLetterOrNumber() || ch == QLatin1Char('_'); };

    int c = std::clamp(pos.col, 0, len - 1);
    int start = c;
    int end = c;
    if (isWordChar(text[c]))
    {
        while (start > 0 && isWordChar(text[start - 1])) --start;
        while (end < len && isWordChar(text[end])) ++end;
    }
    else
    {
        end = c + 1;
    }

    sel_anchor_ = Pos{pos.line, start};
    sel_caret_ = Pos{pos.line, end};
    caret_line_ = pos.line;
    caret_col_ = end;
    selecting_ = false;
    viewport()->update();
}

void ChunkedTextView::keyPressEvent(QKeyEvent* event)
{
    const qint64 total = source_ ? source_->count() : 0;
    if (total <= 0) { QAbstractScrollArea::keyPressEvent(event); return; }

    const bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
    const bool ctrl = event->modifiers().testFlag(Qt::ControlModifier);
    const int visible = visibleLineCount();

    if (ctrl && event->key() == Qt::Key_C) { copySelection(); return; }
    if (ctrl && event->key() == Qt::Key_M) { markSelection(); return; }
    if (ctrl && event->key() == Qt::Key_F) { showSearch(); return; }
    if (event->key() == Qt::Key_Escape && search_active_) { hideSearch(); return; }
    if (event->key() == Qt::Key_F3)
    {
        flushPendingSearch();
        if (shift) findPrevious(); else findNext();
        return;
    }
    if (ctrl && event->key() == Qt::Key_A)
    {
        sel_anchor_ = Pos{0, 0};
        const qint64 last = total - 1;
        sel_caret_ = Pos{last, static_cast<int>(source_->at(last).text.length())};
        caret_line_ = last;
        viewport()->update();
        return;
    }

    switch (event->key())
    {
        case Qt::Key_Up:    moveCaret(caret_line_ - 1, caret_col_, shift); break;
        case Qt::Key_Down:  moveCaret(caret_line_ + 1, caret_col_, shift); break;
        case Qt::Key_Left:  moveCaret(caret_line_, caret_col_ - 1, shift); break;
        case Qt::Key_Right: moveCaret(caret_line_, caret_col_ + 1, shift); break;
        case Qt::Key_PageUp:   moveCaret(caret_line_ - visible, caret_col_, shift); break;
        case Qt::Key_PageDown: moveCaret(caret_line_ + visible, caret_col_, shift); break;
        case Qt::Key_Home:
            if (ctrl) moveCaret(0, 0, shift);
            else moveCaret(caret_line_, 0, shift);
            break;
        case Qt::Key_End:
            if (ctrl) moveCaret(total - 1, static_cast<int>(source_->at(total - 1).text.length()), shift);
            else moveCaret(caret_line_, static_cast<int>(source_->at(caret_line_).text.length()), shift);
            break;
        default:
            QAbstractScrollArea::keyPressEvent(event);
            break;
    }
}

QString ChunkedTextView::selectedText() const
{
    if (!hasSelection() || !source_) return QString();

    Pos start = sel_anchor_;
    Pos end = sel_caret_;
    if (end < start) std::swap(start, end);

    QString result;
    if (start.line == end.line)
    {
        const QString text = source_->at(start.line).text;
        result = text.mid(start.col, end.col - start.col);
    }
    else
    {
        result += source_->at(start.line).text.mid(start.col);
        result += QLatin1Char('\n');
        for (qint64 row = start.line + 1; row < end.line; ++row)
        {
            result += source_->at(row).text;
            result += QLatin1Char('\n');
        }
        result += source_->at(end.line).text.left(end.col);
    }
    return result;
}

void ChunkedTextView::copySelection() const
{
    const QString result = selectedText();
    if (result.isEmpty()) return;

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(result, QClipboard::Clipboard);
    if (clipboard->supportsSelection())
    {
        clipboard->setText(result, QClipboard::Selection);
    }
}

void ChunkedTextView::markSelection()
{
    if (!markings_) return;

    QString text = selectedText();
    const int newline = text.indexOf(QLatin1Char('\n'));
    if (newline >= 0) text = text.left(newline);
    if (text.isEmpty()) return;

    const int existing = markings_->find_marking(text);
    if (existing >= 0)
    {
        markings_->cycle_marking_color(static_cast<uint32_t>(existing));
        return;
    }

    markings_->add_marking(text);
}

void ChunkedTextView::showSearch()
{
    if (!search_widget_)
    {
        search_widget_ = new SearchWidget(viewport());

        search_debounce_ = new QTimer(this);
        search_debounce_->setSingleShot(true);
        search_debounce_->setInterval(kSearchDebounceMs);
        connect(search_debounce_, &QTimer::timeout, this, &ChunkedTextView::runSearch);

        connect(search_widget_, &SearchWidget::queryChanged,
                this, &ChunkedTextView::onSearchQueryChanged);
        connect(search_widget_, &SearchWidget::findNext, this, [this]()
        {
            flushPendingSearch();
            findNext();
        });
        connect(search_widget_, &SearchWidget::findPrevious, this, [this]()
        {
            flushPendingSearch();
            findPrevious();
        });
        connect(search_widget_, &SearchWidget::closeRequested,
                this, &ChunkedTextView::hideSearch);
    }

    search_active_ = true;

    if (hasSelection())
    {
        const QString sel = selectedText();
        if (!sel.contains(QLatin1Char('\n')) && !sel.isEmpty())
            search_widget_->setPattern(sel);
    }

    positionSearchWidget();
    search_widget_->show();
    search_widget_->raise();
    search_widget_->focusInput();

    onSearchQueryChanged();
    flushPendingSearch();
}

void ChunkedTextView::hideSearch()
{
    if (search_debounce_) search_debounce_->stop();
    if (search_widget_) search_widget_->hide();
    search_active_ = false;
    matches_.clear();
    current_match_ = -1;
    viewport()->update();
    setFocus();
}

void ChunkedTextView::positionSearchWidget()
{
    if (!search_widget_) return;
    constexpr int margin = 4;
    search_widget_->adjustSize();
    const int x = viewport()->width() - search_widget_->width() - margin;
    search_widget_->move(std::max(margin, x), margin);
}

void ChunkedTextView::onSearchQueryChanged()
{
    if (!search_widget_) return;
    search_pattern_ = search_widget_->pattern();
    search_regex_ = search_widget_->isRegex();
    search_case_ = search_widget_->isCaseSensitive();
    if (search_debounce_) search_debounce_->start();
}

void ChunkedTextView::flushPendingSearch()
{
    if (search_debounce_ && search_debounce_->isActive())
    {
        search_debounce_->stop();
        runSearch();
    }
}

void ChunkedTextView::runSearch()
{
    matches_.clear();
    current_match_ = -1;

    const qint64 total = source_ ? source_->count() : 0;

    if (!search_active_ || search_pattern_.isEmpty() || total <= 0)
    {
        if (search_widget_)
            search_widget_->setMatchInfo(-1, search_pattern_.isEmpty() ? -1 : 0);
        if (search_widget_) search_widget_->setPatternValid(true);
        viewport()->update();
        return;
    }

    if (search_regex_)
    {
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!search_case_)
            options |= QRegularExpression::CaseInsensitiveOption;

        const QRegularExpression expression(search_pattern_, options);
        if (!expression.isValid())
        {
            if (search_widget_) search_widget_->setPatternValid(false);
            updateSearchInfo();
            viewport()->update();
            return;
        }
        if (search_widget_) search_widget_->setPatternValid(true);

        for (qint64 row = 0; row < total; ++row)
        {
            const QString text = source_->at(row).text;
            QRegularExpressionMatchIterator it = expression.globalMatch(text);
            while (it.hasNext())
            {
                const QRegularExpressionMatch match = it.next();
                if (match.capturedLength() <= 0) continue;
                matches_.append({row,
                    static_cast<int>(match.capturedStart()),
                    static_cast<int>(match.capturedLength())});
            }
        }
    }
    else
    {
        if (search_widget_) search_widget_->setPatternValid(true);
        const Qt::CaseSensitivity sensitivity =
            search_case_ ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const int len = static_cast<int>(search_pattern_.length());

        for (qint64 row = 0; row < total; ++row)
        {
            const QString text = source_->at(row).text;
            int from = 0;
            while (true)
            {
                const int idx = text.indexOf(search_pattern_, from, sensitivity);
                if (idx < 0) break;
                matches_.append({row, idx, len});
                from = idx + len;
            }
        }
    }

    updateSearchInfo();
    viewport()->update();
}

void ChunkedTextView::findNext()
{
    if (matches_.isEmpty()) return;

    if (current_match_ < 0)
    {
        int target = 0;
        for (int i = 0; i < matches_.size(); ++i)
        {
            const Match& match = matches_[i];
            if (match.line > caret_line_ ||
                (match.line == caret_line_ && match.col >= caret_col_))
            {
                target = i;
                break;
            }
        }
        gotoMatch(target);
        return;
    }

    gotoMatch((current_match_ + 1) % matches_.size());
}

void ChunkedTextView::findPrevious()
{
    if (matches_.isEmpty()) return;

    if (current_match_ < 0)
    {
        int target = matches_.size() - 1;
        for (int i = matches_.size() - 1; i >= 0; --i)
        {
            const Match& match = matches_[i];
            if (match.line < caret_line_ ||
                (match.line == caret_line_ && match.col < caret_col_))
            {
                target = i;
                break;
            }
        }
        gotoMatch(target);
        return;
    }

    gotoMatch((current_match_ - 1 + matches_.size()) % matches_.size());
}

void ChunkedTextView::gotoMatch(int index)
{
    if (matches_.isEmpty()) return;

    const int size = matches_.size();
    current_match_ = ((index % size) + size) % size;
    const Match& match = matches_[current_match_];

    sel_anchor_ = Pos{match.line, match.col};
    sel_caret_ = Pos{match.line, match.col + match.len};
    caret_line_ = match.line;
    caret_col_ = match.col + match.len;

    const int visible = visibleLineCount();
    const qint64 target = std::max<qint64>(0, match.line - visible / 2);
    verticalScrollBar()->setValue(static_cast<int>(target));

    updateSearchInfo();
    viewport()->update();
}

void ChunkedTextView::updateSearchInfo()
{
    if (!search_widget_) return;
    if (search_pattern_.isEmpty())
    {
        search_widget_->setMatchInfo(-1, -1);
        return;
    }
    search_widget_->setMatchInfo(current_match_, matches_.size());
}
