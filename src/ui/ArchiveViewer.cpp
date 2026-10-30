#include "ArchiveViewer.hpp"

#include "ArchiveDocument.hpp"
#include "FileViewer.hpp"
#include "LogViewer.hpp"
#include "Logfile.hpp"
#include "ProjectUiManager.hpp"
#include "Translator.hpp"
#include "archive/ArchivePathFilter.hpp"
#include "archive/ZipEntryTreeModel.hpp"
#include "serializer/SerializerLogfile.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProgressBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

ArchiveViewer::ArchiveViewer(QWidget* parent, ArchiveDocument* archive, ProjectUiManager* project_ui)
    : archive_(archive)
    , project_ui_(project_ui)
{
    setParent(parent);

    tabs_ = new QTabWidget(this);
    setupArchiveTab();
    tabs_->setTabsClosable(true);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    tabs_->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    tabs_->tabBar()->installEventFilter(this);

    connect(tabs_, &QTabWidget::tabCloseRequested, this, &ArchiveViewer::onInnerTabCloseRequested);
    connect(tabs_->tabBar(), &QTabBar::customContextMenuRequested, this, &ArchiveViewer::showFileTabContextMenu);
    connect(archive_, &ArchiveDocument::catalogUpdated, this, &ArchiveViewer::onCatalogUpdated);

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(tabs_);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

QString ArchiveViewer::archivePath() const
{
    return archive_ ? archive_->archivePath() : QString();
}

void ArchiveViewer::setupArchiveTab()
{
    archive_tab_ = new QWidget();
    QVBoxLayout* v = new QVBoxLayout(archive_tab_);
    catalog_status_ = new QLabel(Lang::tr("archive.catalog_loading"));
    catalog_progress_ = new QProgressBar();
    catalog_progress_->setRange(0, 100);
    catalog_progress_->setValue(0);
    catalog_progress_->setTextVisible(true);
    filter_edit_ = new QLineEdit();
    filter_edit_->setPlaceholderText(Lang::tr("archive.filter_placeholder"));
    filter_edit_->setClearButtonEnabled(true);
    filter_edit_->setEnabled(false);
    tree_ = new QTreeView();
    tree_model_ = new ZipEntryTreeModel(this);
    tree_->setModel(tree_model_);
    tree_->setHeaderHidden(true);
    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    filter_debounce_ = new QTimer(this);
    filter_debounce_->setSingleShot(true);
    filter_debounce_->setInterval(200);
    connect(filter_debounce_, &QTimer::timeout, this, &ArchiveViewer::applyTreeFilter);

    connect(tree_, &QTreeView::activated, this, &ArchiveViewer::onTreeActivated);
    connect(tree_, &QTreeView::customContextMenuRequested, this, &ArchiveViewer::showTreeContextMenu);
    connect(filter_edit_, &QLineEdit::textChanged, this, &ArchiveViewer::scheduleTreeFilter);

    v->addWidget(catalog_status_);
    v->addWidget(catalog_progress_);
    v->addWidget(filter_edit_);
    v->addWidget(tree_, 1);
    tabs_->addTab(archive_tab_, Lang::tr("archive.tab_title"));
}

void ArchiveViewer::setCatalogProgress(int entries, qint64 bytes_read, qint64 total_bytes)
{
    if (total_bytes > 0)
    {
        const int percent = qBound(0, static_cast<int>(bytes_read * 100 / total_bytes), 99);
        catalog_progress_->setRange(0, 100);
        catalog_progress_->setValue(percent);
        catalog_status_->setText(Lang::tr("archive.catalog_loading_files").arg(entries));
    }
    else
    {
        catalog_progress_->setRange(0, 0);
        catalog_status_->setText(Lang::tr("archive.catalog_loading_files").arg(entries));
    }
}

void ArchiveViewer::onCatalogUpdated()
{
    filter_edit_->setEnabled(true);
    catalog_progress_->hide();

    catalog_paths_ = archive_->catalogEntryPaths();
    catalog_truncated_ = archive_->catalogTruncated();
    applyTreeFilter();

    restorePendingEntries();
}

void ArchiveViewer::scheduleTreeFilter()
{
    filter_debounce_->start();
}

void ArchiveViewer::applyTreeFilter()
{
    const QString pattern = filter_edit_->text().trimmed();
    const QStringList visible =
        logview_archive::filterEntryPaths(catalog_paths_, pattern);
    tree_model_->setEntryPaths(visible);

    if (pattern.isEmpty())
        tree_->expandToDepth(1);
    else
        tree_->expandAll();

    updateCatalogStatusLabel(visible.size());
}

void ArchiveViewer::updateCatalogStatusLabel(int visible_count)
{
    if (catalog_truncated_)
    {
        catalog_status_->setText(Lang::tr("archive.catalog_truncated"));
        return;
    }

    const QString pattern = filter_edit_->text().trimmed();
    if (pattern.isEmpty())
    {
        catalog_status_->setText(Lang::tr("archive.catalog_ready").arg(catalog_paths_.size()));
        return;
    }

    catalog_status_->setText(
        Lang::tr("archive.catalog_filtered").arg(visible_count).arg(catalog_paths_.size()));
}

void ArchiveViewer::restorePendingEntries()
{
    if (!archive_->hasPendingRestore())
        return;

    const std::vector<ArchiveDocument::PendingInnerRestore> pending = archive_->pendingRestores();
    for (const ArchiveDocument::PendingInnerRestore& item : pending)
    {
        QString error;
        Logfile* lf = archive_->openEntry(item.entry_id, &error);
        if (!lf)
            continue;

        serializer::Logfile::deserialize(*lf, item.state);
        addInnerFileViewer(lf, item.entry_id);
    }
    archive_->clearPendingRestores();
}

FileViewer* ArchiveViewer::activeFileViewer() const
{
    if (!tabs_ || tabs_->currentIndex() <= 0)
        return nullptr;
    return qobject_cast<FileViewer*>(tabs_->currentWidget());
}

LogViewer* ArchiveViewer::getDeepestActiveTab()
{
    FileViewer* viewer = activeFileViewer();
    return viewer ? viewer->getDeepestActiveTab() : nullptr;
}

int ArchiveViewer::innerTabIndexForEntry(const QString& entry_id) const
{
    Logfile* lf = archive_->findOpenedLogfile(entry_id);
    if (!lf)
        return -1;
    for (int i = 1; i < tabs_->count(); ++i)
    {
        FileViewer* viewer = qobject_cast<FileViewer*>(tabs_->widget(i));
        if (viewer && viewer->logfile_ == lf)
            return i;
    }
    return -1;
}

void ArchiveViewer::openEntry(const QString& entry_id)
{
    const int existing = innerTabIndexForEntry(entry_id);
    if (existing >= 0)
    {
        tabs_->setCurrentIndex(existing);
        return;
    }

    QString error;
    Logfile* lf = archive_->openEntry(entry_id, &error);
    if (!lf)
    {
        QMessageBox::warning(this, Lang::tr("archive.open_failed.title"),
                             Lang::tr("archive.open_failed").arg(entry_id, error));
        return;
    }

    addInnerFileViewer(lf, entry_id);
}

void ArchiveViewer::addInnerFileViewer(Logfile* lf, const QString& tab_label)
{
    FileViewer* viewer = new FileViewer(tabs_, lf);
    const int index = tabs_->addTab(viewer, tab_label);
    tabs_->setTabToolTip(index, lf->getFileName());
    tabs_->setCurrentIndex(index);

    project_ui_->beginLoadingLogfile(lf, viewer, this);
}

void ArchiveViewer::onInnerTabCloseRequested(int index)
{
    if (index <= 0)
        return;

    FileViewer* viewer = qobject_cast<FileViewer*>(tabs_->widget(index));
    QString entry_id;
    if (viewer && archive_)
    {
        for (const ArchiveDocument::OpenedEntry& entry : archive_->openedEntries())
        {
            if (entry.logfile.get() == viewer->logfile_)
            {
                entry_id = entry.entry_id;
                break;
            }
        }
    }

    tabs_->removeTab(index);
    delete viewer;

    if (!entry_id.isEmpty() && archive_)
        archive_->closeEntry(entry_id);
}

bool ArchiveViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == tabs_->tabBar() && event->type() == QEvent::MouseButtonRelease)
    {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::MiddleButton)
        {
            const int index = tabs_->tabBar()->tabAt(mouse_event->pos());
            if (index > 0)
            {
                onInnerTabCloseRequested(index);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ArchiveViewer::showFileTabContextMenu(const QPoint& pos)
{
    QTabBar* bar = tabs_->tabBar();
    const int index = bar->tabAt(pos);
    if (index <= 0)
        return;

    FileViewer* viewer = qobject_cast<FileViewer*>(tabs_->widget(index));
    QString entry_id;
    if (viewer && archive_)
    {
        for (const ArchiveDocument::OpenedEntry& entry : archive_->openedEntries())
        {
            if (entry.logfile.get() == viewer->logfile_)
            {
                entry_id = entry.entry_id;
                break;
            }
        }
    }

    QMenu menu;
    QAction* copy_name_action = menu.addAction(Lang::tr("tab.copy_file_name"));
    QAction* copy_path_action = menu.addAction(Lang::tr("tab.copy_file_path"));
    QAction* open_folder_action = menu.addAction(Lang::tr("tab.open_folder"));
    menu.addSeparator();
    QAction* reload_action = menu.addAction(Lang::tr("tab.reload"));
    menu.addSeparator();
    QAction* close_action = menu.addAction(Lang::tr("tab.close"));

    const bool has_entry = !entry_id.isEmpty();
    const QString archive_path = archivePath();
    copy_name_action->setEnabled(has_entry);
    copy_path_action->setEnabled(has_entry);
    open_folder_action->setEnabled(!archive_path.isEmpty());
    reload_action->setEnabled(viewer != nullptr && viewer->logfile_ != nullptr);

    QAction* chosen = menu.exec(bar->mapToGlobal(pos));
    if (chosen == reload_action)
        project_ui_->reloadViewer(tabs_, index, this, false);
    else if (chosen == close_action)
        onInnerTabCloseRequested(index);
    else if (chosen == copy_name_action)
        QApplication::clipboard()->setText(QFileInfo(entry_id).fileName());
    else if (chosen == copy_path_action)
        QApplication::clipboard()->setText(QDir::toNativeSeparators(entry_id));
    else if (chosen == open_folder_action)
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(archive_path).absolutePath()));
}

void ArchiveViewer::onTreeActivated(const QModelIndex& index)
{
    const QString entry_id = tree_model_->entryIdForIndex(index);
    if (entry_id.isEmpty())
        return;
    openEntry(entry_id);
}

void ArchiveViewer::showTreeContextMenu(const QPoint& pos)
{
    const QModelIndex index = tree_->indexAt(pos);
    const QString entry_id = tree_model_->entryIdForIndex(index);
    if (entry_id.isEmpty())
        return;

    QMenu menu;
    QAction* open_action = menu.addAction(Lang::tr("archive.open_entry"));
    if (menu.exec(tree_->viewport()->mapToGlobal(pos)) == open_action)
        openEntry(entry_id);
}

ArchiveViewer::~ArchiveViewer()
{
    if (ArchiveDocument* doc = archive_.data())
    {
        disconnect(doc, nullptr, this, nullptr);
        if (project_ui_)
            project_ui_->onArchiveViewerClosing(doc);
    }
    archive_.clear();
}
