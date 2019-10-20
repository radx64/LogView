#include "Viewer.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDebug>
#include <QTextBlock>

#include "TabCompositeViewer.hpp"
#include "ProjectModel.hpp"
#include "GrepNode.hpp"
#include "BookmarksModel.hpp"
#include "GrepNode.hpp"
#include "TextRenderer.hpp"

Viewer::Viewer(QWidget* parent, std::unique_ptr<Logfile> log)
{
    /* project model should be moved outside */
    project_model_ = std::make_unique<ProjectModel>();
    project_model_->file_path_ = log->getFileName();
    project_model_->grep_hierarchy_ = std::make_unique<GrepNode>("ROOT");
    project_model_->logfile_model_ = std::move(log);

    setParent(parent);
    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    bookmarks_widget_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(
        this,
        project_model_->grep_hierarchy_.get(),
        project_model_->logfile_model_->getLines());
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(bookmarks_widget_);
    splitter->addWidget(logViewer_);
    splitter->setSizes({200,1000});

    layout_->addWidget(splitter);
    this->setLayout(layout_);

    bookmarks_widget_->setModel(project_model_->getBookmarksModel());
    connect(bookmarks_widget_, &QListView::doubleClicked, this, &Viewer::bookmarksItemDoubleClicked);
}

Viewer::Viewer(QWidget* parent, std::unique_ptr<ProjectModel> project_model)
{
    setParent(parent);

    project_model_ = std::move(project_model);

    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    bookmarks_widget_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(
        this,
        project_model_->grep_hierarchy_.get(),
        project_model_->logfile_model_->getLines());
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(bookmarks_widget_);
    splitter->addWidget(logViewer_);
    splitter->setSizes({200,1000});

    layout_->addWidget(splitter);
    this->setLayout(layout_);

    bookmarks_widget_->setModel(project_model_->getBookmarksModel());
    connect(bookmarks_widget_, &QListView::doubleClicked, this, &Viewer::bookmarksItemDoubleClicked);
}

TabCompositeViewer* find_deepest_active_tab(TabCompositeViewer* start_point)
{
    if (start_point == nullptr) return start_point;
    const int tab_grep_index = start_point->tabs_->currentIndex();
    qDebug() << "tab_grep_index:" << tab_grep_index;
    QWidget* active_tab = start_point->tabs_->widget(tab_grep_index);
    TabCompositeViewer* active_tab_casted = dynamic_cast<TabCompositeViewer*>(active_tab);
    if (active_tab_casted == nullptr) return start_point;
    TabCompositeViewer* result = find_deepest_active_tab(active_tab_casted);
    return result ? result : start_point;
}

TabCompositeViewer* Viewer::getDeepestActiveTab()
{
    return find_deepest_active_tab(logViewer_);
}

void Viewer::bookmarksItemDoubleClicked(const QModelIndex& idx)
{
    qDebug() << "Doubleclicked item index: " << idx.row();

    Bookmark bookmark = project_model_->getBookmarksModel()->get_bookmark(static_cast<uint32_t>(idx.row()));
    TabCompositeViewer* text_viewer = find_deepest_active_tab(logViewer_);

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
