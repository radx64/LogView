#include "TabCompositeViewer.hpp"

#include "QLabel"
#include "QLayout"
#include "QTabWidget"
#include "QFileDialog"
#include "QMessageBox"
#include "QTextEdit"
#include "QAction"
#include "QMimeData"
#include "QInputDialog"
#include "QDebug"
#include "QTabBar"
#include "QRegularExpression"

TabCompositeViewer::TabCompositeViewer(QWidget* parent) : Viewer(parent)
{
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setTabsClosable(true);
    //Remove close button from "Base" tab;
    tabs_->tabBar()->setTabButton(0, QTabBar::LeftSide, 0);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, 0);

    connect(tabs_, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    tabs_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(tabs_);
    this->setLayout(layout);
}

void TabCompositeViewer::grep(QString pattern)
{
    TabCompositeViewer* viewer = new TabCompositeViewer(this);
    tabs_->addTab(viewer, pattern);

    QRegularExpression exp(pattern);
    qDebug() << exp.pattern();
    qDebug() << lines_;
    QStringList filtered_results;

    for(auto line : this->lines_)
    {
        QRegularExpressionMatch match = exp.match(line);
        if (match.hasMatch()) filtered_results.append(line);
    }

    viewer->text_->setText(filtered_results.join(""));
    viewer->lines_ = filtered_results;
}

void TabCompositeViewer::closeTab(const int index)
{
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}
