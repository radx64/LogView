#include "FileViewer.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDebug>
#include <QTextBlock>

#include "LogViewer.hpp"
#include "ProjectModel.hpp"
#include "GrepNode.hpp"
#include "BookmarksModel.hpp"
#include "GrepNode.hpp"
#include "TextRenderer.hpp"
#include "Logfile.hpp"

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
        logfile_->getLines());
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

    int cursor_offset = 0;

    auto line_it = text_viewer->lines_.begin();
    while(line_it != text_viewer->lines_.end())
    {
        cursor_offset = static_cast<int>(std::distance(text_viewer->lines_.begin(), line_it));
        if (line_it->number >= bookmark.line_number_)
        {
            break;
        }
       ++line_it;
    }

    QTextCursor cursor = text_viewer->text_->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, cursor_offset);
    text_viewer->text_->setTextCursor(cursor);
}
