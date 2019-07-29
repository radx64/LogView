#include "ViewerWidget.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>
#include <QDebug>

#include "BookmarksModel.hpp"
#include "TabCompositeViewer.hpp"
#include "ProjectModel.hpp"
#include "GrepNode.hpp"

ViewerWidget::ViewerWidget(QWidget* parent, std::unique_ptr<Logfile> log)
{
    /* project model should be moved outside */
    project_model_ = std::make_unique<ProjectModel>();
    project_model_->filePath_ = log->getFileName();
    project_model_->grepHierarchy_ = std::make_unique<GrepNode>();
    project_model_->logfile_model_ = std::move(log);

    setParent(parent);
    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    bookmarks_widget_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(this, project_model_->logfile_model_->getLines());
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(bookmarks_widget_);
    splitter->addWidget(logViewer_);
    splitter->setSizes({200,1000});

    layout_->addWidget(splitter);
    this->setLayout(layout_);
    bookmarks_model_ = new BookmarksModel(this);
    bookmarks_widget_->setModel(bookmarks_model_);
    connect(bookmarks_widget_, &QListView::doubleClicked, this, &ViewerWidget::bookmarksItemDoubleClicked);
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

TabCompositeViewer* ViewerWidget::getDeepestActiveTab()
{
    return find_deepest_active_tab(logViewer_);
}

void ViewerWidget::bookmarksItemDoubleClicked(const QModelIndex& idx)
{
    qDebug() << "Doubleclicked item index: " << idx.row();
}
