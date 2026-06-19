#include "SettingsDialog.hpp"

#include <QCheckBox>
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

#include "AutoMarkingsWidget.hpp"
#include "Settings.hpp"

namespace
{
const QColor kDefaultSearchHighlightColor(255, 215, 0, 120);
const QColor kDefaultSearchCurrentMatchColor(255, 140, 0, 200);
}

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
    addSection(tr("Automatic markings"), createAutoMarkingsPage());
    addSection(tr("Updates"), createUpdatesPage());

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

    search_color_button_ = new QPushButton(page);
    search_color_button_->setAutoFillBackground(true);
    connect(search_color_button_, &QPushButton::clicked, this, [this]()
            {
                pickColor(search_color_, tr("Select search highlight color"),
                          kDefaultSearchHighlightColor);
            });

    search_color_reset_ = new QPushButton(tr("Auto"), page);
    search_color_reset_->setToolTip(tr("Use the default search highlight color"));
    connect(search_color_reset_, &QPushButton::clicked, this, [this]()
            {
                search_color_ = QColor();
                updateColorButton();
            });

    QHBoxLayout* searchColorLayout = new QHBoxLayout();
    searchColorLayout->addWidget(search_color_button_, 1);
    searchColorLayout->addWidget(search_color_reset_);

    search_current_color_button_ = new QPushButton(page);
    search_current_color_button_->setAutoFillBackground(true);
    connect(search_current_color_button_, &QPushButton::clicked, this, [this]()
            {
                pickColor(search_current_color_, tr("Select current match color"),
                          kDefaultSearchCurrentMatchColor);
            });

    search_current_color_reset_ = new QPushButton(tr("Auto"), page);
    search_current_color_reset_->setToolTip(tr("Use the default current match color"));
    connect(search_current_color_reset_, &QPushButton::clicked, this, [this]()
            {
                search_current_color_ = QColor();
                updateColorButton();
            });

    QHBoxLayout* searchCurrentColorLayout = new QHBoxLayout();
    searchCurrentColorLayout->addWidget(search_current_color_button_, 1);
    searchCurrentColorLayout->addWidget(search_current_color_reset_);

    form->addRow(tr("Font:"), font_combo_);
    form->addRow(tr("Size:"), font_size_spin_);
    form->addRow(tr("Current line color:"), colorLayout);
    form->addRow(tr("Search highlight color:"), searchColorLayout);
    form->addRow(tr("Current match color:"), searchCurrentColorLayout);

    return page;
}

QWidget* SettingsDialog::createAutoMarkingsPage()
{
    return new AutoMarkingsWidget(this);
}

QWidget* SettingsDialog::createUpdatesPage()
{
    QWidget* page = new QWidget(this);
    QFormLayout* form = new QFormLayout(page);

    check_updates_checkbox_ = new QCheckBox(
        tr("Check for updates on startup"), page);

    QLabel* hint = new QLabel(
        tr("LogView can check GitHub for new releases. You can also check\n"
           "manually from Help \u2192 Check for updates..."),
        page);
    hint->setWordWrap(true);

    form->addRow(check_updates_checkbox_);
    form->addRow(hint);

    return page;
}

void SettingsDialog::loadValues()
{
    const QFont font = Settings::instance().editorFont();
    font_combo_->setCurrentFont(font);
    font_size_spin_->setValue(font.pointSize() > 0 ? font.pointSize() : 10);

    highlight_color_ = Settings::instance().highlightLineColor();
    search_color_ = Settings::instance().searchHighlightColor();
    search_current_color_ = Settings::instance().searchCurrentMatchColor();
    updateColorButton();

    check_updates_checkbox_->setChecked(
        Settings::instance().checkForUpdatesOnStartup());
}

void SettingsDialog::applyColorButtonStyle(QPushButton* button, const QColor& color)
{
    if (color.isValid())
    {
        const QString textColor = color.lightnessF() > 0.5 ? "black" : "white";
        button->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2;")
                .arg(color.name(QColor::HexArgb), textColor));
        button->setText(color.name(QColor::HexArgb));
    }
    else
    {
        button->setStyleSheet(QString());
        button->setText(tr("Automatic"));
    }
}

void SettingsDialog::updateColorButton()
{
    applyColorButtonStyle(highlight_color_button_, highlight_color_);
    applyColorButtonStyle(search_color_button_, search_color_);
    applyColorButtonStyle(search_current_color_button_, search_current_color_);
}

void SettingsDialog::pickColor(QColor& target, const QString& title, const QColor& fallback)
{
    const QColor initial = target.isValid() ? target : fallback;
    const QColor chosen = QColorDialog::getColor(
        initial, this, title, QColorDialog::ShowAlphaChannel);
    if (chosen.isValid())
    {
        target = chosen;
        updateColorButton();
    }
}

void SettingsDialog::pickHighlightColor()
{
    pickColor(highlight_color_, tr("Select current line color"), QColor(Qt::yellow));
}

void SettingsDialog::applyChanges()
{
    QFont font = font_combo_->currentFont();
    font.setPointSize(font_size_spin_->value());

    Settings::instance().setEditorFont(font);
    Settings::instance().setHighlightLineColor(highlight_color_);
    Settings::instance().setSearchHighlightColor(search_color_);
    Settings::instance().setSearchCurrentMatchColor(search_current_color_);
    Settings::instance().setCheckForUpdatesOnStartup(
        check_updates_checkbox_->isChecked());
}
