#include "MergerDialogWindow.hpp"

#include "Translator.hpp"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include "Settings.hpp"

namespace
{
constexpr qint64 kCopyChunkSize = 64 * 1024;
}

MergerDialogWindow::MergerDialogWindow(QWidget* parent) :
    QDialog(parent)
{
    setWindowTitle(Lang::tr("merger.title"));
    setWindowIcon(QIcon(QStringLiteral(":/icon/Add-Files-To-Archive-32.png")));
    setAcceptDrops(true);
    resize(420, 520);

    QVBoxLayout* outer = new QVBoxLayout(this);

    QGroupBox* files_box = new QGroupBox(Lang::tr("merger.files"), this);
    QVBoxLayout* files_layout = new QVBoxLayout(files_box);

    QHBoxLayout* list_row = new QHBoxLayout();

    file_list_ = new QListWidget(files_box);
    file_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    file_list_->setDragDropMode(QAbstractItemView::InternalMove);
    file_list_->setToolTip(Lang::tr("merger.files_tooltip"));
    list_row->addWidget(file_list_);

    QVBoxLayout* arrows = new QVBoxLayout();
    move_up_ = new QToolButton(files_box);
    move_up_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Go-Up-32.png")));
    move_up_->setToolTip(Lang::tr("merger.move_up"));
    move_down_ = new QToolButton(files_box);
    move_down_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Go-Down-32.png")));
    move_down_->setToolTip(Lang::tr("merger.move_down"));
    arrows->addStretch();
    arrows->addWidget(move_up_);
    arrows->addWidget(move_down_);
    arrows->addStretch();
    list_row->addLayout(arrows);

    files_layout->addLayout(list_row);

    QLabel* hint = new QLabel(Lang::tr("merger.drag_hint"), files_box);
    hint->setAlignment(Qt::AlignHCenter);
    files_layout->addWidget(hint);

    QHBoxLayout* buttons_row = new QHBoxLayout();
    add_button_ = new QPushButton(Lang::tr("common.add"), files_box);
    add_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-List-Add-32.png")));
    delete_button_ = new QPushButton(Lang::tr("common.delete"), files_box);
    delete_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-List-Remove-32.png")));
    clear_button_ = new QPushButton(Lang::tr("merger.clear"), files_box);
    clear_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Edit-Clear-32.png")));
    buttons_row->addWidget(add_button_);
    buttons_row->addWidget(delete_button_);
    buttons_row->addWidget(clear_button_);
    buttons_row->addStretch();
    files_layout->addLayout(buttons_row);

    recycle_check_ = new QCheckBox(Lang::tr("merger.recycle"), files_box);
    files_layout->addWidget(recycle_check_);

    outer->addWidget(files_box);

    QLabel* result_label = new QLabel(Lang::tr("merger.result_as_label"), this);
    outer->addWidget(result_label);

    QHBoxLayout* output_row = new QHBoxLayout();
    output_path_ = new QLineEdit(this);
    browse_button_ = new QPushButton(QStringLiteral("..."), this);
    browse_button_->setMaximumWidth(32);
    output_row->addWidget(output_path_);
    output_row->addWidget(browse_button_);
    outer->addLayout(output_row);

    QHBoxLayout* bottom_row = new QHBoxLayout();
    open_result_check_ = new QCheckBox(Lang::tr("merger.open_result"), this);
    open_result_check_->setChecked(true);
    save_button_ = new QPushButton(Lang::tr("merger.save_open"), this);
    save_button_->setIcon(QIcon(QStringLiteral(":/icon/Gnome-Document-Save-32.png")));
    save_button_->setDefault(true);
    cancel_button_ = new QPushButton(Lang::tr("common.cancel"), this);
    bottom_row->addWidget(open_result_check_);
    bottom_row->addStretch();
    bottom_row->addWidget(save_button_);
    bottom_row->addWidget(cancel_button_);
    outer->addLayout(bottom_row);

    connect(move_up_, &QToolButton::clicked, this, &MergerDialogWindow::onMoveUpClicked);
    connect(move_down_, &QToolButton::clicked, this, &MergerDialogWindow::onMoveDownClicked);
    connect(add_button_, &QPushButton::clicked, this, &MergerDialogWindow::onAddClicked);
    connect(delete_button_, &QPushButton::clicked, this, &MergerDialogWindow::onDeleteClicked);
    connect(clear_button_, &QPushButton::clicked, this, &MergerDialogWindow::onClearClicked);
    connect(browse_button_, &QPushButton::clicked, this, &MergerDialogWindow::onBrowseClicked);
    connect(save_button_, &QPushButton::clicked, this, &MergerDialogWindow::onSaveClicked);
    connect(cancel_button_, &QPushButton::clicked, this, &MergerDialogWindow::reject);
    connect(output_path_, &QLineEdit::textEdited, this, &MergerDialogWindow::onOutputEdited);

    QAbstractItemModel* model = file_list_->model();
    connect(model, &QAbstractItemModel::rowsInserted, this, &MergerDialogWindow::updateDefaultOutputPath);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &MergerDialogWindow::updateDefaultOutputPath);
    connect(model, &QAbstractItemModel::rowsMoved, this, &MergerDialogWindow::updateDefaultOutputPath);
    connect(model, &QAbstractItemModel::modelReset, this, &MergerDialogWindow::updateDefaultOutputPath);
}

MergerDialogWindow::~MergerDialogWindow() = default;

void MergerDialogWindow::addFiles(const QStringList& paths)
{
    for (const QString& path : paths)
        appendFile(path);
}

QString MergerDialogWindow::outputPath() const
{
    return output_path_->text();
}

bool MergerDialogWindow::shouldOpenResult() const
{
    return open_result_check_->isChecked();
}

void MergerDialogWindow::appendFile(const QString& path)
{
    if (path.isEmpty())
        return;

    const QString native = QDir::toNativeSeparators(path);
    QListWidgetItem* item = new QListWidgetItem(native, file_list_);
    item->setData(Qt::UserRole, path);
    item->setToolTip(native);
}

QStringList MergerDialogWindow::collectFiles() const
{
    QStringList files;
    for (int row = 0; row < file_list_->count(); ++row)
        files << file_list_->item(row)->data(Qt::UserRole).toString();
    return files;
}

void MergerDialogWindow::onAddClicked()
{
    const QStringList paths = QFileDialog::getOpenFileNames(this,
        Lang::tr("merger.add_files_title"),
        Settings::instance().lastOpenedFileDirectory(),
        Lang::tr("filter.all_files"));
    if (paths.isEmpty())
        return;

    Settings::instance().setLastOpenedFileDirectory(
        QFileInfo(paths.constFirst()).absolutePath());

    addFiles(paths);
}

void MergerDialogWindow::onDeleteClicked()
{
    const QList<QListWidgetItem*> selected = file_list_->selectedItems();
    for (QListWidgetItem* item : selected)
        delete file_list_->takeItem(file_list_->row(item));
}

void MergerDialogWindow::onClearClicked()
{
    file_list_->clear();
}

void MergerDialogWindow::onMoveUpClicked()
{
    const int row = file_list_->currentRow();
    if (row <= 0)
        return;
    QListWidgetItem* item = file_list_->takeItem(row);
    file_list_->insertItem(row - 1, item);
    file_list_->setCurrentItem(item);
}

void MergerDialogWindow::onMoveDownClicked()
{
    const int row = file_list_->currentRow();
    if (row < 0 || row >= file_list_->count() - 1)
        return;
    QListWidgetItem* item = file_list_->takeItem(row);
    file_list_->insertItem(row + 1, item);
    file_list_->setCurrentItem(item);
}

void MergerDialogWindow::onBrowseClicked()
{
    QString start_dir = Settings::instance().lastOpenedFileDirectory();
    if (!output_path_->text().isEmpty())
        start_dir = QFileInfo(output_path_->text()).absolutePath();

    const QString path = QFileDialog::getSaveFileName(this,
        Lang::tr("merger.result_as_title"),
        start_dir,
        Lang::tr("filter.log_files"));
    if (path.isEmpty())
        return;

    output_user_edited_ = true;
    output_path_->setText(QDir::toNativeSeparators(path));
}

void MergerDialogWindow::onOutputEdited()
{
    output_user_edited_ = !output_path_->text().isEmpty();
    if (!output_user_edited_)
        updateDefaultOutputPath();
}

QString MergerDialogWindow::defaultOutputFor(const QString& firstFile)
{
    const QFileInfo info(firstFile);
    const QString suffix = info.suffix();
    QString name = info.completeBaseName() + QStringLiteral("_merged");
    if (!suffix.isEmpty())
        name += QLatin1Char('.') + suffix;
    return info.absoluteDir().filePath(name);
}

void MergerDialogWindow::updateDefaultOutputPath()
{
    if (output_user_edited_)
        return;

    if (file_list_->count() == 0)
    {
        output_path_->clear();
        return;
    }

    const QString first = file_list_->item(0)->data(Qt::UserRole).toString();
    output_path_->setText(QDir::toNativeSeparators(defaultOutputFor(first)));
}

void MergerDialogWindow::onSaveClicked()
{
    const QStringList inputs = collectFiles();
    if (inputs.isEmpty())
    {
        QMessageBox::warning(this, Lang::tr("merger.title"),
            Lang::tr("merger.need_file"));
        return;
    }

    const QString output = output_path_->text().trimmed();
    if (output.isEmpty())
    {
        QMessageBox::warning(this, Lang::tr("merger.title"),
            Lang::tr("merger.need_dest"));
        return;
    }

    const QString output_absolute = QFileInfo(output).absoluteFilePath();
    for (const QString& input : inputs)
    {
        if (QFileInfo(input).absoluteFilePath() == output_absolute)
        {
            QMessageBox::warning(this, Lang::tr("merger.title"),
                Lang::tr("merger.dest_is_source"));
            return;
        }
    }

    if (!mergeFiles(inputs, output))
        return;

    if (recycle_check_->isChecked())
    {
        for (const QString& input : inputs)
            QFile::moveToTrash(input);
    }

    Settings::instance().setLastOpenedFileDirectory(
        QFileInfo(output).absolutePath());

    output_path_->setText(QDir::toNativeSeparators(output));
    accept();
}

bool MergerDialogWindow::mergeFiles(const QStringList& inputs, const QString& output)
{
    QFile out_file(output);
    if (!out_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, Lang::tr("merger.title"),
            Lang::tr("file.write_failed").arg(output));
        return false;
    }

    for (const QString& input : inputs)
    {
        QFile in_file(input);
        if (!in_file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, Lang::tr("merger.title"),
                Lang::tr("file.read_failed").arg(input));
            return false;
        }

        char last_byte = '\n';
        while (!in_file.atEnd())
        {
            const QByteArray chunk = in_file.read(kCopyChunkSize);
            if (chunk.isEmpty())
                break;
            if (out_file.write(chunk) != chunk.size())
            {
                QMessageBox::warning(this, Lang::tr("merger.title"),
                    Lang::tr("merger.write_error").arg(output));
                return false;
            }
            last_byte = chunk.at(chunk.size() - 1);
        }

        if (last_byte != '\n')
            out_file.write("\n", 1);
    }

    return true;
}

void MergerDialogWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MergerDialogWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (!mime->hasUrls())
        return;

    for (const QUrl& url : mime->urls())
    {
        const QString local = url.toLocalFile();
        if (!local.isEmpty() && QFileInfo(local).isFile())
            appendFile(local);
    }
    event->acceptProposedAction();
}
