#include "ViewerWidget.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>
#include <QSplitter>

#include "TabCompositeViewer.hpp"

ViewerWidget::ViewerWidget(QWidget* parent)
{
    setParent(parent);

    layout_ = new QHBoxLayout();
    bookmarks_ = new QListView();
    bookmarks_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(this);
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QSplitter* splitter = new QSplitter();
    splitter->addWidget(bookmarks_);
    splitter->addWidget(logViewer_);

    layout_->addWidget(splitter);
    this->setLayout(layout_);

    /* PoC mockup */
    QStandardItemModel* iStandardModel = new QStandardItemModel(this);
    QList<QStandardItem*>* items = new QList<QStandardItem*> ();
    QStandardItem* item = new QStandardItem();
    item->setText("Bookmark A");
    item->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
    items->append(item);

    item = new QStandardItem();
    item->setText("Bookmark B");
    item->setIcon(this->style()->standardIcon(QStyle::SP_DialogCloseButton));
    items->append(item);

    iStandardModel->appendColumn(*items);
    bookmarks_->setModel(iStandardModel);
}
