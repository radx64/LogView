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

TabCompositeViewer::TabCompositeViewer(QWidget* parent) : Viewer(parent)
{
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setTabsClosable(true);
    //Remove close button from "Base" tab;
    tabs_->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

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
    Lines filtered_results;
    for(auto line : lines_)
    {
        QRegularExpressionMatch match = exp.match(line.text);
        if (match.hasMatch())
        {
            filtered_results.append({line.number, line.text});
            qDebug() << line.number << " " <<line.text;
        }
    }

    viewer->setContent(filtered_results);
}

QString linesToQString(const Lines& lines)
{
    QString result;
    for (const auto& line : lines)
    {
        result.append(QString::number(line.number).rightJustified(5,' ') + " | " + line.text);
    }
    return result;
}

void TabCompositeViewer::setContent(const Lines& lines)
{
    lines_ = lines;
    text_->setText(linesToQString(lines_));
}

void TabCompositeViewer::closeTab(const int index)
{
    QWidget* tabContents = tabs_->widget(index);
    tabs_->removeTab(index);
    if (tabContents != nullptr) delete(tabContents);
}
