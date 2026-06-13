#include "FileViewer.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDebug>
#include <QTextBlock>
#include <QMenu>
#include <QAction>

#include "LogViewer.hpp"
#include "ProjectModel.hpp"
#include "GrepNode.hpp"
#include "BookmarksModel.hpp"
#include "GrepNode.hpp"
#include "ChunkedTextView.hpp"
#include "LineSource.hpp"
#include "Logfile.hpp"
#include "Bookmark.hpp"
#include "BookmarkDialogWindow.hpp"

FileViewer::FileViewer(QWidget* parent, Logfile* logfile)
{
    setParent(parent);
    logfile_ = logfile;

    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    bookmarks_widget_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new LogViewer(
        this,
        logfile_->grep_hierarchy_.get(),
        logfile_->getLineSource());
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(bookmarks_widget_);
    splitter->addWidget(logViewer_);
    splitter->setSizes({200,1000});

    layout_->addWidget(splitter);
    this->setLayout(layout_);

    bookmarks_widget_->viewport()->installEventFilter(this);
    bookmarks_widget_->setModel(logfile_->getBookmarksModel());
    bookmarks_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarks_widget_, &QListView::doubleClicked, this, &FileViewer::bookmarksItemDoubleClicked);
    connect(bookmarks_widget_, &QListView::customContextMenuRequested, this, &FileViewer::showBookmarksContextMenu);
}

FileViewer::~FileViewer()
{
    //if(on_destroy_action_) on_destroy_action_();
    emit destroyed(logfile_);
    qDebug() << "File closed!";
}

LogViewer* find_deepest_active_tab(LogViewer* start_point)
{
    if (start_point == nullptr) return start_point;
    const int tab_grep_index = start_point->tabs_->currentIndex();
    QWidget* active_tab = start_point->tabs_->widget(tab_grep_index);
    LogViewer* active_tab_casted = dynamic_cast<LogViewer*>(active_tab);
    if (active_tab_casted == nullptr) return start_point;
    LogViewer* result = find_deepest_active_tab(active_tab_casted);
    return result ? result : start_point;
}

LogViewer* FileViewer::getDeepestActiveTab()
{
    return find_deepest_active_tab(logViewer_);
}

bool FileViewer::eventFilter(QObject *obj, QEvent *event)
{
    (void) obj;
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent * mouseEvent = static_cast <QMouseEvent *> (event);
        if (mouseEvent->button() == Qt::RightButton) return true;
    }
    return false;
}

void FileViewer::bookmarksItemDoubleClicked(const QModelIndex& idx)
{
    Bookmark bookmark = logfile_->getBookmarksModel()->get_bookmark(static_cast<uint32_t>(idx.row()));
    LogViewer* text_viewer = find_deepest_active_tab(logViewer_);

    const qint64 row = text_viewer->source()->rowForLineNumber(bookmark.line_number_);
    text_viewer->text_->gotoRow(row);
}

void FileViewer::showBookmarksContextMenu(const QPoint& pos)
{
    const QModelIndex index = bookmarks_widget_->indexAt(pos);
    if (!index.isValid()) return;
    const uint32_t row = static_cast<uint32_t>(index.row());

    QMenu menu;
    QAction* edit_action = menu.addAction(tr("Edit..."));
    QAction* delete_action = menu.addAction(tr("Delete"));
    QAction* chosen = menu.exec(bookmarks_widget_->viewport()->mapToGlobal(pos));
    if (chosen == nullptr) return;

    BookmarksModel* model = logfile_->getBookmarksModel();

    if (chosen == delete_action)
    {
        model->remove_bookmark(row);
        return;
    }

    if (chosen == edit_action)
    {
        const Bookmark bookmark = model->get_bookmark(row);
        BookmarkDialogWindow dialog(this);
        dialog.setWindowTitle(tr("Edit bookmark"));
        dialog.setName(bookmark.text_);
        dialog.setIcon(bookmark.icon_);
        if (dialog.exec() != QDialog::Accepted) return;

        const auto result = dialog.getResult();
        model->update_bookmark(row, result.icon, result.name);
    }
}
