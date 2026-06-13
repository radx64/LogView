#include "SettingsDialog.hpp"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "Settings.hpp"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Options"));
    resize(640, 420);

    section_tree_ = new QTreeWidget(this);
    section_tree_->setHeaderHidden(true);
    section_tree_->setMaximumWidth(200);
    section_tree_->setMinimumWidth(150);

    pages_ = new QStackedWidget(this);

    connect(section_tree_, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* current, QTreeWidgetItem*)
            {
                if (!current) return;
                const int index = current->data(0, Qt::UserRole).toInt();
                pages_->setCurrentIndex(index);
            });

    addSection(tr("Editor"), createEditorPage());

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->addWidget(section_tree_);
    contentLayout->addWidget(pages_, 1);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
        this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]()
            {
                applyChanges();
                accept();
            });
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::applyChanges);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(contentLayout, 1);
    mainLayout->addWidget(buttons);

    loadValues();

    if (section_tree_->topLevelItemCount() > 0)
        section_tree_->setCurrentItem(section_tree_->topLevelItem(0));
}

void SettingsDialog::addSection(const QString& title, QWidget* page)
{
    const int index = pages_->addWidget(page);
    QTreeWidgetItem* item = new QTreeWidgetItem(section_tree_);
    item->setText(0, title);
    item->setData(0, Qt::UserRole, index);
}

QWidget* SettingsDialog::createEditorPage()
{
    QWidget* page = new QWidget(this);
    QFormLayout* form = new QFormLayout(page);

    font_combo_ = new QFontComboBox(page);

    font_size_spin_ = new QSpinBox(page);
    font_size_spin_->setRange(6, 72);
    font_size_spin_->setSuffix(tr(" pt"));

    highlight_color_button_ = new QPushButton(page);
    highlight_color_button_->setAutoFillBackground(true);
    connect(highlight_color_button_, &QPushButton::clicked,
            this, &SettingsDialog::pickHighlightColor);

    highlight_color_reset_ = new QPushButton(tr("Auto"), page);
    highlight_color_reset_->setToolTip(tr("Derive the highlight color from the active theme"));
    connect(highlight_color_reset_, &QPushButton::clicked, this, [this]()
            {
                highlight_color_ = QColor();
                updateColorButton();
            });

    QHBoxLayout* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(highlight_color_button_, 1);
    colorLayout->addWidget(highlight_color_reset_);

    form->addRow(tr("Font:"), font_combo_);
    form->addRow(tr("Size:"), font_size_spin_);
    form->addRow(tr("Current line color:"), colorLayout);

    return page;
}

void SettingsDialog::loadValues()
{
    const QFont font = Settings::instance().editorFont();
    font_combo_->setCurrentFont(font);
    font_size_spin_->setValue(font.pointSize() > 0 ? font.pointSize() : 10);

    highlight_color_ = Settings::instance().highlightLineColor();
    updateColorButton();
}

void SettingsDialog::updateColorButton()
{
    if (highlight_color_.isValid())
    {
        const QString textColor =
            highlight_color_.lightnessF() > 0.5 ? "black" : "white";
        highlight_color_button_->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2;")
                .arg(highlight_color_.name(QColor::HexArgb), textColor));
        highlight_color_button_->setText(highlight_color_.name(QColor::HexArgb));
    }
    else
    {
        highlight_color_button_->setStyleSheet(QString());
        highlight_color_button_->setText(tr("Automatic"));
    }
}

void SettingsDialog::pickHighlightColor()
{
    const QColor initial = highlight_color_.isValid() ? highlight_color_ : QColor(Qt::yellow);
    const QColor chosen = QColorDialog::getColor(
        initial, this, tr("Select current line color"),
        QColorDialog::ShowAlphaChannel);
    if (chosen.isValid())
    {
        highlight_color_ = chosen;
        updateColorButton();
    }
}

void SettingsDialog::applyChanges()
{
    QFont font = font_combo_->currentFont();
    font.setPointSize(font_size_spin_->value());

    Settings::instance().setEditorFont(font);
    Settings::instance().setHighlightLineColor(highlight_color_);
}
