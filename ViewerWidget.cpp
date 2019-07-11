#include "ViewerWidget.hpp"

#include <QHBoxLayout>
#include <QListView>
#include <QSizePolicy>
#include <QIcon>
#include <QStandardItemModel>

#include "TabCompositeViewer.hpp"

ViewerWidget::ViewerWidget(QWidget* parent) : parent_(parent)
{
    layout_ = new QHBoxLayout();
    bookmarks_ = new QListView();
    bookmarks_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(this);
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout_->addWidget(bookmarks_);
    layout_->addWidget(logViewer_);
    this->setLayout(layout_);

    /* PoC mockup */
    //bookmarks_->addItems(QStringList{"bookmark A","bookmark B"});

    QStandardItemModel* iStandardModel = new QStandardItemModel(this);
    QList<QStandardItem*>* items = new QList<QStandardItem*> ();
    QStandardItem* item = new QStandardItem();
    item->setText("Bookmark A");
    item->setIcon(QIcon::fromTheme("appointment-new"));
    items->append(item);
    iStandardModel->appendColumn(*items);
    bookmarks_->setModel(iStandardModel);

}
