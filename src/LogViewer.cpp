#include "LogViewer.hpp"

#include "QLayout"
#include "QTabWidget"
#include "QTabBar"

#include "ChunkedTextView.hpp"
#include "FilteredLineSource.hpp"
#include "GrepNode.hpp"
#include "LineSource.hpp"

LogViewer::LogViewer(QWidget* parent, GrepNode* grep_node, std::shared_ptr<LineSource> source,
                     MarkingsModel* markings)
    : source_(std::move(source)), grep_node_(grep_node), markings_(markings)
{
    text_ = new ChunkedTextView(parent, source_.get(), markings_);
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setTabsClosable(true);

    //Remove close button from "Base" tab;
    tabs_->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

    connect(tabs_, &QTabWidget::tabCloseRequested, this, &LogViewer::closeTab);
    tabs_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(tabs_);
    this->setLayout(layout);
}

QString generateTabName(const GrepNode* grep, const QString base_name)
{
    QString tabName = base_name + " (";
    tabName += grep->isRegEx() ? "R" : "r";
    tabName += grep->isCaseInsensitive() ? "C" : "c";
    tabName += grep->isInverted() ? "I" : "i";
    tabName += ")";
    return tabName;
}

LogViewer* LogViewer::grep(GrepNode* grep)
{
    const QString pattern = QString::fromStdString(grep->getPattern());

    std::shared_ptr<LineSource> filtered = FilteredLineSource::create(source_, grep);

    LogViewer* viewer = new LogViewer(this, grep, std::move(filtered), markings_);
    tabs_->addTab(viewer, generateTabName(grep, pattern));
    return viewer;
}
void LogViewer::closeTab(const int index)
{
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);

    //tab with index 0 points is like a parent (Base) so it is skipped in grep hierarchy storage
    auto child_to_be_removed = grep_node_->getChildren()[index - 1];
    grep_node_->removeChild(child_to_be_removed);
}

GrepNode* LogViewer::getGrepNode()
{
    return grep_node_;
}
