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

QString generateTabName(const QString& base_name, bool is_regex, bool is_case_insensitive)
{
    if (is_regex) return base_name + " (Rgx)";
    if (is_case_insensitive) return  base_name + " (In)";
    return base_name;
}

TabCompositeViewer* TabCompositeViewer::grep(QString pattern, bool is_regex, bool is_case_insensitive)
{
    Lines filtered_results;
    if (is_regex)
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
            if(line.text.contains(pattern, is_case_insensitive ? Qt::CaseInsensitive : Qt::CaseSensitive))
            {
               filtered_results.append({line.number, line.text});
            }
        }
    }

    GrepNode* new_grep_node = new GrepNode(pattern.toStdString());
    grep_node_->addChild(new_grep_node);

    TabCompositeViewer* viewer = new TabCompositeViewer(this, new_grep_node, filtered_results);
    tabs_->addTab(viewer, generateTabName(pattern, is_regex, is_case_insensitive));
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
