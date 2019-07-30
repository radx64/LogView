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

void TabCompositeViewer::grep(QString pattern)
{
    QRegularExpression exp(pattern);
    Lines filtered_results;
    for(auto line : lines_)
    {
        QRegularExpressionMatch match = exp.match(line.text);
        if (match.hasMatch())
        {
            filtered_results.append({line.number, line.text});
        }
    }

    GrepNode* new_grep_node = new GrepNode(pattern.toStdString());
    grep_node_->addChild(new_grep_node);

    TabCompositeViewer* viewer = new TabCompositeViewer(this, new_grep_node, filtered_results);
    tabs_->addTab(viewer, pattern);

}
void TabCompositeViewer::closeTab(const int index)
{
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}
