#include "MarkingDialogWindow.hpp"

#include "Marking.hpp"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
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

    QFormLayout* form = new QFormLayout();
    form->addRow(new QLabel(tr("Text"), this), text_edit_);
    form->addRow(new QLabel(tr("Color"), this), color_button_);

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
    }
}

void MarkingDialogWindow::updateColorButton()
{
    const QString textColor = color_.lightnessF() > 0.5 ? "black" : "white";
    color_button_->setStyleSheet(
        QStringLiteral("background-color: %1; color: %2;")
            .arg(color_.name(), textColor));
    color_button_->setText(color_.name());
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
    }
}

MarkingDialogWindow::Result MarkingDialogWindow::getResult() const
{
    Result result;
    result.text = text_edit_->text();
    result.color = color_.isValid()
                       ? color_.name()
                       : QString::fromLatin1(marking_colors::kDefaultColor);
    return result;
}
