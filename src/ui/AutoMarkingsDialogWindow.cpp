#include "AutoMarkingsDialogWindow.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableView>
#include <QVBoxLayout>

#include "AutoMarkingsModel.hpp"

namespace
{
class AutoMarkingRuleDialog : public QDialog
{
public:
    explicit AutoMarkingRuleDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(tr("Automatic marking"));
        resize(460, 180);

        pattern_edit_ = new QLineEdit(this);
        pattern_edit_->setPlaceholderText(tr("Example: ERR/.*"));
        connect(pattern_edit_, &QLineEdit::textEdited, this,
                [this](const QString&) { updatePatternPalette(); });

        enabled_check_ = new QCheckBox(tr("Enabled"), this);
        enabled_check_->setChecked(true);

        case_sensitive_check_ = new QCheckBox(tr("Case sensitive"), this);
        case_sensitive_check_->setChecked(true);

        text_color_button_ = new QPushButton(this);
        text_color_button_->setAutoFillBackground(true);
        connect(text_color_button_, &QPushButton::clicked, this,
                [this]() { pickTextColor(); });

        background_color_button_ = new QPushButton(this);
        background_color_button_->setAutoFillBackground(true);
        connect(background_color_button_, &QPushButton::clicked, this,
                [this]() { pickBackgroundColor(); });

        clear_background_button_ = new QPushButton(tr("None"), this);
        connect(clear_background_button_, &QPushButton::clicked, this,
                [this]()
                {
                    background_color_ = QColor();
                    updateColorButtons();
                });

        QHBoxLayout* background_layout = new QHBoxLayout();
        background_layout->setContentsMargins(0, 0, 0, 0);
        background_layout->addWidget(background_color_button_, 1);
        background_layout->addWidget(clear_background_button_);

        QFormLayout* form = new QFormLayout();
        form->addRow(tr("Regex:"), pattern_edit_);
        form->addRow(tr("Font color:"), text_color_button_);
        form->addRow(tr("Background:"), background_layout);
        form->addRow(enabled_check_);
        form->addRow(case_sensitive_check_);

        buttons_ = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttons_, &QDialogButtonBox::accepted, this,
                [this]() { validateAndAccept(); });
        connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);

        error_label_ = new QLabel(this);
        error_label_->setWordWrap(true);
        error_label_->setStyleSheet(QStringLiteral("color: #c62828;"));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addLayout(form);
        layout->addWidget(error_label_);
        layout->addWidget(buttons_);

        text_color_ = QColor(QStringLiteral("#1565c0"));
        updateColorButtons();
        updatePatternPalette();
    }

    void setMarking(const AutoMarking& marking)
    {
        pattern_edit_->setText(marking.pattern_);
        enabled_check_->setChecked(marking.enabled_);
        case_sensitive_check_->setChecked(marking.case_sensitive_);
        text_color_ = QColor(marking.text_color_);
        if (!text_color_.isValid())
            text_color_ = QColor(QStringLiteral("#1565c0"));
        background_color_ = QColor(marking.background_color_);
        updateColorButtons();
        updatePatternPalette();
    }

    AutoMarking marking() const
    {
        return AutoMarking(pattern_edit_->text(),
                           text_color_.name(),
                           background_color_.isValid()
                               ? background_color_.name()
                               : QString(),
                           enabled_check_->isChecked(),
                           case_sensitive_check_->isChecked());
    }

private:
    void pickTextColor()
    {
        const QColor chosen = QColorDialog::getColor(
            text_color_.isValid() ? text_color_ : QColor(Qt::blue),
            this, tr("Select font color"));
        if (chosen.isValid())
        {
            text_color_ = chosen;
            updateColorButtons();
        }
    }

    void pickBackgroundColor()
    {
        const QColor chosen = QColorDialog::getColor(
            background_color_.isValid() ? background_color_ : QColor(Qt::yellow),
            this, tr("Select background color"));
        if (chosen.isValid())
        {
            background_color_ = chosen;
            updateColorButtons();
        }
    }

    void updateColorButtons()
    {
        const auto setButton = [](QPushButton* button, const QColor& color,
                                  const QString& emptyText)
        {
            if (!color.isValid())
            {
                button->setStyleSheet(QString());
                button->setText(emptyText);
                return;
            }

            const QString label_color = color.lightnessF() > 0.5 ? "black" : "white";
            button->setStyleSheet(
                QStringLiteral("QPushButton { background-color: %1; color: %2;"
                               " border: 1px solid palette(mid); padding: 4px; }")
                    .arg(color.name(), label_color));
            button->setText(color.name());
        };

        setButton(text_color_button_, text_color_, tr("Pick font color"));
        setButton(background_color_button_, background_color_, tr("No background"));
    }

    void validateAndAccept()
    {
        const QString pattern = pattern_edit_->text();
        if (pattern.isEmpty())
        {
            error_label_->setText(tr("Regex cannot be empty."));
            return;
        }

        const QRegularExpression expression(pattern);
        if (!expression.isValid())
        {
            error_label_->setText(
                tr("Invalid regex: %1").arg(expression.errorString()));
            return;
        }

        accept();
    }

    void updatePatternPalette()
    {
        QPalette palette = pattern_edit_->palette();
        const QString pattern = pattern_edit_->text();
        const QRegularExpression expression(pattern);
        const QColor background = (!pattern.isEmpty() && expression.isValid())
                                      ? QColor(Qt::green).lighter()
                                      : QColor(Qt::red).lighter();
        palette.setColor(QPalette::Base, background);
        palette.setColor(QPalette::Text, Qt::black);
        pattern_edit_->setPalette(palette);

        if (expression.isValid())
            error_label_->clear();
    }

    QLineEdit* pattern_edit_{nullptr};
    QCheckBox* enabled_check_{nullptr};
    QCheckBox* case_sensitive_check_{nullptr};
    QPushButton* text_color_button_{nullptr};
    QPushButton* background_color_button_{nullptr};
    QPushButton* clear_background_button_{nullptr};
    QDialogButtonBox* buttons_{nullptr};
    QLabel* error_label_{nullptr};
    QColor text_color_{};
    QColor background_color_{};
};
} // namespace

AutoMarkingsDialogWindow::AutoMarkingsDialogWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Automatic markings"));
    resize(760, 420);

    QLabel* hint = new QLabel(
        tr("These regex templates are saved in %1 and are applied to every opened log file.")
            .arg(QFileInfo(AutoMarkingsModel::filePath()).fileName()),
        this);
    hint->setWordWrap(true);

    table_ = new QTableView(this);
    table_->setModel(&AutoMarkingsModel::instance());
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(AutoMarkingsModel::PatternColumn,
                                                     QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);

    connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() { updateButtonState(); });
    connect(table_, &QTableView::doubleClicked,
            this, [this](const QModelIndex&) { editSelectedMarking(); });

    QPushButton* add_button = new QPushButton(tr("Add..."), this);
    edit_button_ = new QPushButton(tr("Edit..."), this);
    remove_button_ = new QPushButton(tr("Delete"), this);

    connect(add_button, &QPushButton::clicked, this,
            [this]() { addMarking(); });
    connect(edit_button_, &QPushButton::clicked, this,
            [this]() { editSelectedMarking(); });
    connect(remove_button_, &QPushButton::clicked, this,
            [this]() { removeSelectedMarking(); });

    QHBoxLayout* actions = new QHBoxLayout();
    actions->addWidget(add_button);
    actions->addWidget(edit_button_);
    actions->addWidget(remove_button_);
    actions->addStretch();

    QDialogButtonBox* buttons =
        new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(hint);
    layout->addWidget(table_, 1);
    layout->addLayout(actions);
    layout->addWidget(buttons);

    updateButtonState();
}

int AutoMarkingsDialogWindow::selectedRow() const
{
    const QModelIndex index = table_->currentIndex();
    return index.isValid() ? index.row() : -1;
}

void AutoMarkingsDialogWindow::addMarking()
{
    AutoMarkingRuleDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;
    AutoMarkingsModel::instance().add_marking(dialog.marking());
}

void AutoMarkingsDialogWindow::editSelectedMarking()
{
    const int row = selectedRow();
    if (row < 0) return;

    AutoMarkingRuleDialog dialog(this);
    dialog.setWindowTitle(tr("Edit automatic marking"));
    dialog.setMarking(AutoMarkingsModel::instance().get_marking(row));
    if (dialog.exec() != QDialog::Accepted) return;

    AutoMarkingsModel::instance().update_marking(row, dialog.marking());
}

void AutoMarkingsDialogWindow::removeSelectedMarking()
{
    const int row = selectedRow();
    if (row < 0) return;
    AutoMarkingsModel::instance().remove_marking(row);
}

void AutoMarkingsDialogWindow::updateButtonState()
{
    const bool has_selection = selectedRow() >= 0;
    edit_button_->setEnabled(has_selection);
    remove_button_->setEnabled(has_selection);
}
