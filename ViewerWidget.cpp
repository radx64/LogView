#include "ViewerWidget.hpp"

#include <QHBoxLayout>
#include <QListWidget>
#include <QSizePolicy>

#include "TabCompositeViewer.hpp"

ViewerWidget::ViewerWidget(QWidget* parent) : parent_(parent)
{
    layout_ = new QHBoxLayout();
    bookmarks_ = new QListWidget();
    bookmarks_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    logViewer_ = new TabCompositeViewer(this);
    logViewer_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout_->addWidget(bookmarks_);
    layout_->addWidget(logViewer_);
    this->setLayout(layout_);

    /* PoC mockup */
    bookmarks_->addItems(QStringList{"bookmark A","bookmark B"});
    /* */
}
