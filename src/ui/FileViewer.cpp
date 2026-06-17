#include "FileViewer.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
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
#include "MarkingsModel.hpp"
#include "GrepNode.hpp"
#include "ChunkedTextView.hpp"
#include "LineSource.hpp"
#include "Logfile.hpp"
#include "Bookmark.hpp"
#include "Marking.hpp"
#include "BookmarkDialogWindow.hpp"
#include "MarkingDialogWindow.hpp"

FileViewer::FileViewer(QWidget* parent, Logfile* logfile)
{
    setParent(parent);
    logfile_ = logfile;

    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    markings_widget_ = new QListView();

    logViewer_ = new LogViewer(
        this,
        logfile_->grep_hierarchy_.get(),
        logfile_->getLineSource(),
        logfile_->getMarkingsModel());
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QWidget* bookmarks_section = new QWidget();
    QVBoxLayout* bookmarks_layout = new QVBoxLayout(bookmarks_section);
    bookmarks_layout->setContentsMargins(0, 0, 0, 0);
    bookmarks_layout->addWidget(new QLabel(tr("Bookmarks")));
    bookmarks_layout->addWidget(bookmarks_widget_, 1);

    QWidget* markings_section = new QWidget();
    QVBoxLayout* markings_layout = new QVBoxLayout(markings_section);
    markings_layout->setContentsMargins(0, 0, 0, 0);
    markings_layout->addWidget(new QLabel(tr("Markings")));
    markings_layout->addWidget(markings_widget_, 1);

    QSplitter* side_panel = new QSplitter(Qt::Vertical);
    side_panel->addWidget(bookmarks_section);
    side_panel->addWidget(markings_section);
    side_panel->setSizes({500, 500});

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(side_panel);
    splitter->addWidget(logViewer_);
    splitter->setSizes({200,1000});

    layout_->addWidget(splitter);
    this->setLayout(layout_);

    bookmarks_widget_->viewport()->installEventFilter(this);
    bookmarks_widget_->setModel(logfile_->getBookmarksModel());
    bookmarks_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarks_widget_, &QListView::doubleClicked, this, &FileViewer::bookmarksItemDoubleClicked);
    connect(bookmarks_widget_, &QListView::customContextMenuRequested, this, &FileViewer::showBookmarksContextMenu);

    markings_widget_->viewport()->installEventFilter(this);
    markings_widget_->setModel(logfile_->getMarkingsModel());
    markings_widget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    markings_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(markings_widget_, &QListView::doubleClicked, this, &FileViewer::markingsItemDoubleClicked);
    connect(markings_widget_, &QListView::customContextMenuRequested, this, &FileViewer::showMarkingsContextMenu);
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

void FileViewer::markingsItemDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid()) return;
    const uint32_t row = static_cast<uint32_t>(idx.row());

    MarkingsModel* model = logfile_->getMarkingsModel();
    const Marking marking = model->get_marking(row);

    MarkingDialogWindow dialog(this);
    dialog.setWindowTitle(tr("Edit marking"));
    dialog.setText(marking.text_);
    dialog.setColor(marking.color_);
    dialog.setTextColor(marking.text_color_);
    if (dialog.exec() != QDialog::Accepted) return;

    const auto result = dialog.getResult();
    model->update_marking(row, result.text, result.color, result.text_color);
}

void FileViewer::showMarkingsContextMenu(const QPoint& pos)
{
    const QModelIndex index = markings_widget_->indexAt(pos);
    if (!index.isValid()) return;
    const uint32_t row = static_cast<uint32_t>(index.row());

    QMenu menu;
    QAction* edit_action = menu.addAction(tr("Edit..."));
    QAction* delete_action = menu.addAction(tr("Delete"));
    QAction* chosen = menu.exec(markings_widget_->viewport()->mapToGlobal(pos));
    if (chosen == nullptr) return;

    MarkingsModel* model = logfile_->getMarkingsModel();

    if (chosen == delete_action)
    {
        model->remove_marking(row);
        return;
    }

    if (chosen == edit_action)
    {
        const Marking marking = model->get_marking(row);
        MarkingDialogWindow dialog(this);
        dialog.setWindowTitle(tr("Edit marking"));
        dialog.setText(marking.text_);
        dialog.setColor(marking.color_);
        dialog.setTextColor(marking.text_color_);
        if (dialog.exec() != QDialog::Accepted) return;

        const auto result = dialog.getResult();
        model->update_marking(row, result.text, result.color, result.text_color);
    }
}
