#include "MarkingDialogWindow.hpp"

#include "Marking.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

MarkingDialogWindow::MarkingDialogWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Marking"));
    resize(400, 120);

    text_edit_ = new QLineEdit(this);

    color_button_ = new QPushButton(this);
    color_button_->setAutoFillBackground(true);
    connect(color_button_, &QPushButton::clicked, this, &MarkingDialogWindow::pickColor);

    color_ = QColor(QString::fromLatin1(marking_colors::kDefaultColor));
    updateColorButton();

    text_color_button_ = new QPushButton(this);
    text_color_button_->setAutoFillBackground(true);
    connect(text_color_button_, &QPushButton::clicked, this, &MarkingDialogWindow::pickTextColor);

    auto_text_color_ = new QCheckBox(tr("Automatic"), this);
    auto_text_color_->setChecked(true);
    auto_text_color_->setToolTip(tr("Pick a readable font color based on the marking color"));
    connect(auto_text_color_, &QCheckBox::toggled, this, &MarkingDialogWindow::updateTextColorButton);

    updateTextColorButton();

    QHBoxLayout* text_color_layout = new QHBoxLayout();
    text_color_layout->setContentsMargins(0, 0, 0, 0);
    text_color_layout->addWidget(auto_text_color_);
    text_color_layout->addWidget(text_color_button_, 1);
    QWidget* text_color_row = new QWidget(this);
    text_color_row->setLayout(text_color_layout);

    QFormLayout* form = new QFormLayout();
    form->addRow(new QLabel(tr("Text"), this), text_edit_);
    form->addRow(new QLabel(tr("Color"), this), color_button_);
    form->addRow(new QLabel(tr("Font color"), this), text_color_row);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    text_edit_->setFocus();
}

void MarkingDialogWindow::pickColor()
{
    const QColor chosen = QColorDialog::getColor(
        color_.isValid() ? color_ : QColor(Qt::yellow),
        this, tr("Select marking color"));
    if (chosen.isValid())
    {
        color_ = chosen;
        updateColorButton();
        updateTextColorButton();
    }
}

void MarkingDialogWindow::pickTextColor()
{
    const QColor initial = text_color_.isValid()
                               ? text_color_
                               : QColor(marking_colors::contrastingTextColor(color_.name()));
    const QColor chosen = QColorDialog::getColor(initial, this, tr("Select font color"));
    if (chosen.isValid())
    {
        text_color_ = chosen;
        auto_text_color_->setChecked(false);
        updateTextColorButton();
    }
}

void MarkingDialogWindow::updateColorButton()
{
    const QString textColor = color_.lightnessF() > 0.5 ? "black" : "white";
    color_button_->setStyleSheet(
        QStringLiteral("QPushButton { background-color: %1; color: %2;"
                       " border: 1px solid palette(mid); padding: 4px; }")
            .arg(color_.name(), textColor));
    color_button_->setText(color_.name());
}

void MarkingDialogWindow::updateTextColorButton()
{
    const bool automatic = auto_text_color_->isChecked();
    text_color_button_->setEnabled(!automatic);

    const QColor effective = (!automatic && text_color_.isValid())
                                 ? text_color_
                                 : QColor(marking_colors::contrastingTextColor(color_.name()));

    text_color_button_->setStyleSheet(
        QStringLiteral("QPushButton { background-color: %1; color: %2;"
                       " border: 1px solid palette(mid); padding: 4px; }")
            .arg(color_.name(), effective.name()));
    text_color_button_->setText(automatic ? tr("Auto (%1)").arg(effective.name())
                                           : effective.name());
}

void MarkingDialogWindow::setText(const QString& text)
{
    text_edit_->setText(text);
    text_edit_->selectAll();
}

void MarkingDialogWindow::setColor(const QString& color)
{
    const QColor parsed(color);
    if (parsed.isValid())
    {
        color_ = parsed;
        updateColorButton();
        updateTextColorButton();
    }
}

void MarkingDialogWindow::setTextColor(const QString& textColor)
{
    const QColor parsed(textColor);
    const bool automatic = textColor.isEmpty() || !parsed.isValid();
    if (!automatic) text_color_ = parsed;
    auto_text_color_->setChecked(automatic);
    updateTextColorButton();
}

MarkingDialogWindow::Result MarkingDialogWindow::getResult() const
{
    Result result;
    result.text = text_edit_->text();
    result.color = color_.isValid()
                       ? color_.name()
                       : QString::fromLatin1(marking_colors::kDefaultColor);
    result.text_color = (!auto_text_color_->isChecked() && text_color_.isValid())
                            ? text_color_.name()
                            : QString();
    return result;
}
