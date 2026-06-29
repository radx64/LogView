#pragma once

#include <QFrame>

class QLineEdit;
class QToolButton;
class QLabel;

class SearchWidget : public QFrame
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget* parent = nullptr);

    QString pattern() const;
    bool isRegex() const;
    bool isCaseSensitive() const;

    void setPattern(const QString& text);
    void focusInput();

    // total < 0 clears the label, total == 0 shows "No results",
    // current < 0 shows the match count, otherwise shows "current/total".
    void setMatchInfo(int current, int total);
    void setSearching(int percent);
    void setPatternValid(bool valid);

signals:
    void queryChanged();
    void findNext();
    void findPrevious();
    void closeRequested();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QLineEdit* input_{nullptr};
    QToolButton* regex_button_{nullptr};
    QToolButton* case_button_{nullptr};
    QToolButton* prev_button_{nullptr};
    QToolButton* next_button_{nullptr};
    QToolButton* close_button_{nullptr};
    QLabel* info_label_{nullptr};
};
