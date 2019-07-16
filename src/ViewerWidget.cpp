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

ViewerWidget::ViewerWidget(QWidget* parent)
{
    setParent(parent);

    layout_ = new QHBoxLayout();
    bookmarks_widget_ = new QListView();
    bookmarks_widget_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(this);
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

void ViewerWidget::bookmarksItemDoubleClicked(const QModelIndex& idx)
{
    qDebug() << "Doubleclicked item index: " << idx.row();
}
