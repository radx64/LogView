#include "ArchiveViewer.hpp"

#include "ArchiveDocument.hpp"
#include "FileViewer.hpp"
#include "LogViewer.hpp"
#include "Logfile.hpp"
#include "ProjectUiManager.hpp"
#include "Translator.hpp"
#include "archive/ZipEntryTreeModel.hpp"
#include "serializer/SerializerLogfile.hpp"

#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QTabBar>
#include <QTabWidget>
#include <QTreeView>
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

    connect(tabs_, &QTabWidget::tabCloseRequested, this, &ArchiveViewer::onInnerTabCloseRequested);
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
    tree_ = new QTreeView();
    tree_model_ = new ZipEntryTreeModel(this);
    tree_->setModel(tree_model_);
    tree_->setHeaderHidden(true);
    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(tree_, &QTreeView::activated, this, &ArchiveViewer::onTreeActivated);
    connect(tree_, &QTreeView::customContextMenuRequested, this, &ArchiveViewer::showTreeContextMenu);

    v->addWidget(catalog_status_);
    v->addWidget(tree_, 1);
    tabs_->addTab(archive_tab_, Lang::tr("archive.tab_title"));
}

void ArchiveViewer::onCatalogUpdated()
{
    tree_model_->setEntryPaths(archive_->catalogEntryPaths());
    tree_->expandAll();

    if (archive_->catalogTruncated())
        catalog_status_->setText(Lang::tr("archive.catalog_truncated"));
    else
        catalog_status_->setText(Lang::tr("archive.catalog_ready").arg(archive_->catalogEntryPaths().size()));

    restorePendingEntries();
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
