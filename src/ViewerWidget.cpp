#include "ViewerWidget.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>

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

    /* PoC mockup */
    bookmarks_model_ = new BookmarksModel(this);
//    QList<QStandardItem*>* items = new QList<QStandardItem*>();
//    QStandardItem* item = new QStandardItem();
//    item->setText("Bookmark A");
//    item->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
//    items->append(item);

//    item = new QStandardItem();
//    item->setText("Bookmark B");
//    item->setIcon(this->style()->standardIcon(QStyle::SP_DialogCloseButton));
//    items->append(item);
//    bookmarks_model_->appendColumn(*items);
    /* PoC mockup */
    bookmarks_widget_->setModel(bookmarks_model_);
}
