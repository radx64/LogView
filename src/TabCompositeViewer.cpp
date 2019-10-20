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

#include "Logfile.hpp"
#include "TextRenderer.hpp"
#include "LineNumberingBasedOnModelPolicy.hpp"
#include "GrepNode.hpp"

TabCompositeViewer::TabCompositeViewer(QWidget* parent, GrepNode* grep_node, const Lines lines)
    : lines_(lines), grep_node_(grep_node)
{
    std::unique_ptr<ILineNumberingPolicy> lineNumberingPolicy = std::make_unique<LineNumberingBasedOnModelPolicy>(lines_);
    text_ = new TextRenderer(parent, lines, std::move(lineNumberingPolicy));
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setTabsClosable(true);
    //Remove close button from "Base" tab;
    tabs_->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

    connect(tabs_, &QTabWidget::tabCloseRequested, this, &TabCompositeViewer::closeTab);
    tabs_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(tabs_);
    this->setLayout(layout);
}

QString generateTabName(const GrepNode* grep, const QString base_name)
{
    if (grep->isRegEx()) return base_name + " (Rgx)";
    if (grep->isCaseInsensitive()) return  base_name + " (In)";
    return base_name;
}

TabCompositeViewer* TabCompositeViewer::grep(GrepNode* grep)
{
    const QString pattern = QString().fromStdString(grep->getPattern());

    Lines filtered_results;
    if (grep->isRegEx())
    {
        QRegularExpression exp(pattern);
        for(auto line : lines_)
        {
            QRegularExpressionMatch match = exp.match(line.text);
            if (match.hasMatch()) filtered_results.append({line.number, line.text});
        }
    }
    else
    {
        for(auto line : lines_)
        {
            if(line.text.contains(pattern, grep->isCaseInsensitive() ? Qt::CaseInsensitive : Qt::CaseSensitive))
            {
               filtered_results.append({line.number, line.text});
            }
        }
    }

    bool is_inverted = false; //todo add inversion functionality

    TabCompositeViewer* viewer = new TabCompositeViewer(this, grep, filtered_results);
    tabs_->addTab(viewer, generateTabName(grep, pattern));
    return viewer;
}
void TabCompositeViewer::closeTab(const int index)
{
    qDebug() << "Closing tab " << index;
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);

    //tab with index 0 points is like a parent (Base) so it is skipped in grep hierarchy storage
    auto child_to_be_removed = grep_node_->getChildren()[index - 1];
    grep_node_->removeChild(child_to_be_removed);
}

GrepNode* TabCompositeViewer::getGrepNode()
{
    return grep_node_;
}
