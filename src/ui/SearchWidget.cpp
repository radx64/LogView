#include "SearchWidget.hpp"

#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

SearchWidget::SearchWidget(QWidget* parent) :
    QFrame(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    setLineWidth(2);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    input_ = new QLineEdit(this);
    input_->setPlaceholderText(tr("Find"));
    input_->setClearButtonEnabled(true);
    input_->setMinimumWidth(260);
    input_->installEventFilter(this);
    layout->addWidget(input_);

    info_label_ = new QLabel(this);
    info_label_->setMinimumWidth(128);
    info_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(info_label_);

    regex_button_ = new QToolButton(this);
    regex_button_->setText(QStringLiteral(".*"));
    regex_button_->setCheckable(true);
    regex_button_->setToolTip(tr("Regular expression"));
    layout->addWidget(regex_button_);

    case_button_ = new QToolButton(this);
    case_button_->setText(QStringLiteral("Aa"));
    case_button_->setCheckable(true);
    case_button_->setToolTip(tr("Case sensitive"));
    layout->addWidget(case_button_);

    prev_button_ = new QToolButton(this);
    prev_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Go-Up-32.png")));
    prev_button_->setToolTip(tr("Previous match (Shift+F3)"));
    layout->addWidget(prev_button_);

    next_button_ = new QToolButton(this);
    next_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Go-Down-32.png")));
    next_button_->setToolTip(tr("Next match (F3)"));
    layout->addWidget(next_button_);

    close_button_ = new QToolButton(this);
    close_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Window-Close-32.png")));
    close_button_->setToolTip(tr("Close (Esc)"));
    layout->addWidget(close_button_);

    connect(input_, &QLineEdit::textChanged, this, &SearchWidget::queryChanged);
    connect(input_, &QLineEdit::returnPressed, this, &SearchWidget::findNext);
    connect(regex_button_, &QToolButton::toggled, this, &SearchWidget::queryChanged);
    connect(case_button_, &QToolButton::toggled, this, &SearchWidget::queryChanged);
    connect(prev_button_, &QToolButton::clicked, this, &SearchWidget::findPrevious);
    connect(next_button_, &QToolButton::clicked, this, &SearchWidget::findNext);
    connect(close_button_, &QToolButton::clicked, this, &SearchWidget::closeRequested);
}

QString SearchWidget::pattern() const
{
    return input_->text();
}

bool SearchWidget::isRegex() const
{
    return regex_button_->isChecked();
}

bool SearchWidget::isCaseSensitive() const
{
    return case_button_->isChecked();
}

void SearchWidget::setPattern(const QString& text)
{
    input_->setText(text);
}

void SearchWidget::focusInput()
{
    input_->setFocus();
    input_->selectAll();
}

void SearchWidget::setMatchInfo(int current, int total)
{
    QPalette pal = info_label_->palette();
    pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));

    if (total < 0)
    {
        info_label_->clear();
    }
    else if (total == 0)
    {
        pal.setColor(QPalette::WindowText, QColor(200, 60, 60));
        info_label_->setText(tr("No results"));
    }
    else if (current < 0)
    {
        info_label_->setText(tr("%n match(es)", nullptr, total));
    }
    else
    {
        info_label_->setText(QStringLiteral("%1/%2").arg(current + 1).arg(total));
    }
    info_label_->setPalette(pal);
}

void SearchWidget::setPatternValid(bool valid)
{
    if (!regex_button_->isChecked())
    {
        input_->setPalette(QPalette());
        return;
    }

    QPalette pal = input_->palette();
    const QColor background = valid ? QColor(Qt::green).lighter()
                                    : QColor(Qt::red).lighter();
    pal.setColor(QPalette::Base, background);
    pal.setColor(QPalette::Text, Qt::black);
    input_->setPalette(pal);
}

bool SearchWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == input_ && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        switch (key_event->key())
        {
            case Qt::Key_Escape:
                emit closeRequested();
                return true;
            case Qt::Key_F3:
                if (key_event->modifiers().testFlag(Qt::ShiftModifier))
                    emit findPrevious();
                else
                    emit findNext();
                return true;
            default:
                break;
        }
    }
    return QFrame::eventFilter(watched, event);
}
