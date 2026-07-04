#include "AutoBookmarksWidget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#include "AutoBookmark.hpp"
#include "AutoBookmarksModel.hpp"
#include "Translator.hpp"

namespace
{
struct IconEntry
{
    const char* label;
    const char* path;
};

const IconEntry kIcons[] = {
    {"Bookmark", ":/icon/Gnome-Bookmark-New-32.png"},
    {"Warning", ":/icon/Gnome-Dialog-Warning-32.png"},
    {"Indent", ":/icon/Gnome-Format-Indent-More-32.png"},
    {"Log", ":/icon/Gnome-Logviewer-32.png"},
    {"Apply", ":/icon/Dialog-Apply.png"},
    {"Open", ":/icon/Gnome-Document-Open-32.png"},
    {"Save", ":/icon/Gnome-Document-Save-32.png"},
};

const char* kDefaultIcon = ":/icon/Gnome-Bookmark-New-32.png";

class AutoBookmarkRuleDialog : public QDialog
{
public:
    explicit AutoBookmarkRuleDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(Lang::tr("autobm.dialog.title"));
        resize(480, 320);

        name_edit_ = new QLineEdit(this);
        name_edit_->setPlaceholderText(Lang::tr("autobm.name_placeholder"));

        description_edit_ = new QPlainTextEdit(this);
        description_edit_->setMaximumHeight(60);

        pattern_edit_ = new QLineEdit(this);
        pattern_edit_->setPlaceholderText(Lang::tr("autobm.pattern_placeholder"));
        connect(pattern_edit_, &QLineEdit::textEdited, this,
                [this](const QString&) { updatePatternPalette(); });

        icon_combo_ = new QComboBox(this);
        for (const auto& entry : kIcons)
        {
            const QString path = QString::fromLatin1(entry.path);
            icon_combo_->addItem(QIcon(path), QString::fromLatin1(entry.label), path);
        }

        tags_edit_ = new QLineEdit(this);
        tags_edit_->setPlaceholderText(Lang::tr("autobm.tags_placeholder"));

        enabled_check_ = new QCheckBox(Lang::tr("common.enabled"), this);
        enabled_check_->setChecked(true);

        case_sensitive_check_ = new QCheckBox(Lang::tr("common.case_sensitive"), this);
        case_sensitive_check_->setChecked(true);

        QFormLayout* form = new QFormLayout();
        form->addRow(Lang::tr("common.name_label"), name_edit_);
        form->addRow(Lang::tr("autobm.description_label"), description_edit_);
        form->addRow(Lang::tr("common.regex_label"), pattern_edit_);
        form->addRow(Lang::tr("common.icon_label"), icon_combo_);
        form->addRow(Lang::tr("common.tags_label"), tags_edit_);
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

        updatePatternPalette();
    }

    void setRule(const AutoBookmark& rule)
    {
        name_edit_->setText(rule.name_);
        description_edit_->setPlainText(rule.description_);
        pattern_edit_->setText(rule.pattern_);
        const int icon_index = icon_combo_->findData(rule.icon_);
        if (icon_index >= 0) icon_combo_->setCurrentIndex(icon_index);
        tags_edit_->setText(rule.tags_.join(QStringLiteral(", ")));
        enabled_check_->setChecked(rule.enabled_);
        case_sensitive_check_->setChecked(rule.case_sensitive_);
        updatePatternPalette();
    }

    AutoBookmark rule() const
    {
        QString icon = icon_combo_->currentData().toString();
        if (icon.isEmpty()) icon = QString::fromLatin1(kDefaultIcon);

        QStringList tags;
        const QStringList raw = tags_edit_->text().split(QChar(','), Qt::SkipEmptyParts);
        for (const QString& tag : raw)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty() && !tags.contains(trimmed))
                tags.append(trimmed);
        }

        return AutoBookmark(name_edit_->text().trimmed(),
                            description_edit_->toPlainText().trimmed(),
                            pattern_edit_->text(),
                            icon,
                            tags,
                            enabled_check_->isChecked(),
                            case_sensitive_check_->isChecked());
    }

private:
    void validateAndAccept()
    {
        if (name_edit_->text().trimmed().isEmpty())
        {
            error_label_->setText(Lang::tr("common.name_empty"));
            return;
        }

        const QString pattern = pattern_edit_->text();
        if (pattern.isEmpty())
        {
            error_label_->setText(Lang::tr("common.regex_empty"));
            return;
        }

        const QRegularExpression expression(pattern);
        if (!expression.isValid())
        {
            error_label_->setText(
                Lang::tr("common.invalid_regex").arg(expression.errorString()));
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

    QLineEdit* name_edit_{nullptr};
    QPlainTextEdit* description_edit_{nullptr};
    QLineEdit* pattern_edit_{nullptr};
    QComboBox* icon_combo_{nullptr};
    QLineEdit* tags_edit_{nullptr};
    QCheckBox* enabled_check_{nullptr};
    QCheckBox* case_sensitive_check_{nullptr};
    QDialogButtonBox* buttons_{nullptr};
    QLabel* error_label_{nullptr};
};
} // namespace

AutoBookmarksWidget::AutoBookmarksWidget(QWidget* parent)
    : QWidget(parent)
{
    QLabel* hint = new QLabel(
        Lang::tr("autobm.hint"),
        this);
    hint->setWordWrap(true);

    file_list_ = new QListWidget(this);
    file_list_->setMaximumWidth(200);
    connect(file_list_, &QListWidget::currentRowChanged, this,
            [this](int) { refreshRuleTable(); updateButtonState(); });

    QPushButton* add_file_button = new QPushButton(Lang::tr("autobm.new_file"), this);
    remove_file_button_ = new QPushButton(Lang::tr("autobm.delete_file"), this);
    connect(add_file_button, &QPushButton::clicked, this, [this]() { addFile(); });
    connect(remove_file_button_, &QPushButton::clicked, this,
            [this]() { removeSelectedFile(); });

    QHBoxLayout* file_buttons = new QHBoxLayout();
    file_buttons->addWidget(add_file_button);
    file_buttons->addWidget(remove_file_button_);

    QVBoxLayout* file_column = new QVBoxLayout();
    file_column->addWidget(new QLabel(Lang::tr("autobm.rule_files_label"), this));
    file_column->addWidget(file_list_, 1);
    file_column->addLayout(file_buttons);

    rule_model_ = new QStandardItemModel(this);
    rule_table_ = new QTableView(this);
    rule_table_->setModel(rule_model_);
    rule_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    rule_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    rule_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rule_table_->horizontalHeader()->setStretchLastSection(true);
    rule_table_->verticalHeader()->setVisible(false);
    connect(rule_table_, &QTableView::doubleClicked, this,
            [this](const QModelIndex&) { editSelectedRule(); });

    add_rule_button_ = new QPushButton(Lang::tr("common.add_ellipsis"), this);
    edit_rule_button_ = new QPushButton(Lang::tr("common.edit_ellipsis"), this);
    remove_rule_button_ = new QPushButton(Lang::tr("common.delete"), this);
    connect(add_rule_button_, &QPushButton::clicked, this, [this]() { addRule(); });
    connect(edit_rule_button_, &QPushButton::clicked, this,
            [this]() { editSelectedRule(); });
    connect(remove_rule_button_, &QPushButton::clicked, this,
            [this]() { removeSelectedRule(); });

    QHBoxLayout* rule_buttons = new QHBoxLayout();
    rule_buttons->addWidget(add_rule_button_);
    rule_buttons->addWidget(edit_rule_button_);
    rule_buttons->addWidget(remove_rule_button_);
    rule_buttons->addStretch();

    QVBoxLayout* rule_column = new QVBoxLayout();
    rule_column->addWidget(new QLabel(Lang::tr("autobm.rules_label"), this));
    rule_column->addWidget(rule_table_, 1);
    rule_column->addLayout(rule_buttons);

    QHBoxLayout* columns = new QHBoxLayout();
    columns->addLayout(file_column);
    columns->addLayout(rule_column, 1);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(hint);
    layout->addLayout(columns, 1);

    connect(&AutoBookmarksModel::instance(), &AutoBookmarksModel::filesChanged,
            this, [this]() { refreshFileList(); });

    refreshFileList();
}

QString AutoBookmarksWidget::currentFile() const
{
    QListWidgetItem* item = file_list_->currentItem();
    return item ? item->text() : QString();
}

int AutoBookmarksWidget::selectedRuleRow() const
{
    const QModelIndex index = rule_table_->currentIndex();
    return index.isValid() ? index.row() : -1;
}

void AutoBookmarksWidget::refreshFileList()
{
    const QString previous = currentFile();
    file_list_->clear();
    const QStringList files = AutoBookmarksModel::instance().files();
    file_list_->addItems(files);

    int row = files.indexOf(previous);
    if (row < 0 && !files.isEmpty()) row = 0;
    if (row >= 0) file_list_->setCurrentRow(row);

    refreshRuleTable();
    updateButtonState();
}

void AutoBookmarksWidget::refreshRuleTable()
{
    rule_model_->clear();
    rule_model_->setHorizontalHeaderLabels(
        {Lang::tr("common.enabled"), Lang::tr("common.name"), Lang::tr("common.regex"),
         Lang::tr("common.icon"), Lang::tr("common.tags"), Lang::tr("common.case_sensitive")});

    const QString file = currentFile();
    if (file.isEmpty()) return;

    const QVector<AutoBookmark> rules = AutoBookmarksModel::instance().rules(file);
    for (const AutoBookmark& rule : rules)
    {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(rule.enabled_ ? Lang::tr("common.yes") : Lang::tr("common.no")));
        QStandardItem* name_item = new QStandardItem(QIcon(rule.icon_), rule.name_);
        row.append(name_item);
        row.append(new QStandardItem(rule.pattern_));
        row.append(new QStandardItem(rule.icon_));
        row.append(new QStandardItem(rule.tags_.join(QStringLiteral(", "))));
        row.append(new QStandardItem(rule.case_sensitive_ ? Lang::tr("common.yes") : Lang::tr("common.no")));
        rule_model_->appendRow(row);
    }
}

void AutoBookmarksWidget::addFile()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, Lang::tr("autobm.new_rule_file"), Lang::tr("autobm.file_name"), QLineEdit::Normal,
        QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    const QString created = AutoBookmarksModel::instance().createFile(name);
    if (created.isEmpty())
    {
        QMessageBox::warning(this, Lang::tr("autobm.new_rule_file"),
                             Lang::tr("autobm.file_exists"));
        return;
    }

    refreshFileList();
    const int row = AutoBookmarksModel::instance().files().indexOf(created);
    if (row >= 0) file_list_->setCurrentRow(row);
}

void AutoBookmarksWidget::removeSelectedFile()
{
    const QString file = currentFile();
    if (file.isEmpty()) return;

    if (QMessageBox::question(
            this, Lang::tr("autobm.delete_rule_file"),
            Lang::tr("autobm.delete_confirm").arg(file)) != QMessageBox::Yes)
        return;

    AutoBookmarksModel::instance().deleteFile(file);
    refreshFileList();
}

void AutoBookmarksWidget::addRule()
{
    const QString file = currentFile();
    if (file.isEmpty()) return;

    AutoBookmarkRuleDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return;
    AutoBookmarksModel::instance().addRule(file, dialog.rule());
    refreshRuleTable();
}

void AutoBookmarksWidget::editSelectedRule()
{
    const QString file = currentFile();
    const int row = selectedRuleRow();
    if (file.isEmpty() || row < 0) return;

    AutoBookmarkRuleDialog dialog(this);
    dialog.setWindowTitle(Lang::tr("autobm.edit_title"));
    dialog.setRule(AutoBookmarksModel::instance().rule(file, row));
    if (dialog.exec() != QDialog::Accepted) return;
    AutoBookmarksModel::instance().updateRule(file, row, dialog.rule());
    refreshRuleTable();
}

void AutoBookmarksWidget::removeSelectedRule()
{
    const QString file = currentFile();
    const int row = selectedRuleRow();
    if (file.isEmpty() || row < 0) return;
    AutoBookmarksModel::instance().removeRule(file, row);
    refreshRuleTable();
}

void AutoBookmarksWidget::updateButtonState()
{
    const bool has_file = !currentFile().isEmpty();
    remove_file_button_->setEnabled(has_file);
    add_rule_button_->setEnabled(has_file);
    edit_rule_button_->setEnabled(has_file);
    remove_rule_button_->setEnabled(has_file);
}
