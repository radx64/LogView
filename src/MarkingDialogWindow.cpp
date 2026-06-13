#include "MarkingDialogWindow.hpp"

#include "Marking.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

MarkingDialogWindow::MarkingDialogWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Marking"));
    resize(400, 120);

    text_edit_ = new QLineEdit(this);
    color_combo_ = new QComboBox(this);
    populateColors();

    QFormLayout* form = new QFormLayout();
    form->addRow(new QLabel(tr("Text"), this), text_edit_);
    form->addRow(new QLabel(tr("Color"), this), color_combo_);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    text_edit_->setFocus();
}

void MarkingDialogWindow::populateColors()
{
    for (int i = 0; i < marking_colors::kPaletteSize; ++i)
    {
        const auto& entry = marking_colors::kPalette[i];
        const QString hex = QString::fromLatin1(entry.hex);
        color_combo_->addItem(marking_color_icon(hex),
                              QString::fromLatin1(entry.name), hex);
    }
}

void MarkingDialogWindow::setText(const QString& text)
{
    text_edit_->setText(text);
    text_edit_->selectAll();
}

void MarkingDialogWindow::setColor(const QString& color)
{
    const int index = color_combo_->findData(color);
    if (index >= 0)
    {
        color_combo_->setCurrentIndex(index);
        return;
    }

    // Color is not part of the palette: add it as a custom entry.
    color_combo_->addItem(marking_color_icon(color), tr("Custom"), color);
    color_combo_->setCurrentIndex(color_combo_->count() - 1);
}

MarkingDialogWindow::Result MarkingDialogWindow::getResult() const
{
    Result result;
    result.text = text_edit_->text();
    result.color = color_combo_->currentData().toString();
    if (result.color.isEmpty())
        result.color = QString::fromLatin1(marking_colors::kDefaultColor);
    return result;
}
