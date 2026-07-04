#include "LogViewer.hpp"

#include "Translator.hpp"

#include "QLayout"
#include "QTabWidget"
#include "QTabBar"
#include "QMouseEvent"
#include "QMenu"
#include "QApplication"
#include "QClipboard"

#include "ChunkedTextView.hpp"
#include "FilteredLineSource.hpp"
#include "GrepNode.hpp"
#include "LineSource.hpp"
#include "loader/BackgroundTask.hpp"

LogViewer::LogViewer(QWidget* parent, GrepNode* grep_node, std::shared_ptr<LineSource> source,
                     MarkingsModel* markings)
    : source_(std::move(source)), grep_node_(grep_node), markings_(markings)
{
    text_ = new ChunkedTextView(parent, source_, markings_);
    tabs_ = new QTabWidget();
    tabs_->addTab(text_,"Base");
    tabs_->setTabsClosable(true);

    //Remove close button from "Base" tab;
    tabs_->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
    tabs_->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

    connect(tabs_, &QTabWidget::tabCloseRequested, this, &LogViewer::closeTab);
    tabs_->tabBar()->installEventFilter(this);
    tabs_->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabs_->tabBar(), &QTabBar::customContextMenuRequested, this, &LogViewer::showTabContextMenu);
    tabs_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(tabs_);
    this->setLayout(layout);
}

QString generateTabName(const GrepNode* grep, const QString base_name)
{
    QString tabName = base_name + " (";
    tabName += grep->isRegEx() ? "R" : "r";
    tabName += grep->isCaseSensitive() ? "C" : "c";
    tabName += grep->isInverted() ? "I" : "i";
    tabName += ")";
    return tabName;
}

LogViewer* LogViewer::grep(GrepNode* grep, std::function<void(LogViewer*)> on_ready)
{
    const QString pattern = QString::fromStdString(grep->getPattern());
    const QString base_name = generateTabName(grep, pattern);

    auto filtered = std::make_shared<FilteredLineSource>(source_, grep);

    LogViewer* viewer = new LogViewer(this, grep, filtered, markings_);
    const int new_index = tabs_->addTab(viewer, base_name);
    tabs_->setCurrentIndex(new_index);

    startFilter(viewer, std::move(filtered), base_name, std::move(on_ready));
    return viewer;
}

void LogViewer::startFilter(LogViewer* child, std::shared_ptr<FilteredLineSource> source,
                            const QString& base_name, std::function<void(LogViewer*)> on_ready)
{
    BackgroundTask* task = new BackgroundTask(
        [source](const BackgroundTask::ProgressFn& report) { return source->compute(report); },
        child);

    connect(task, &BackgroundTask::progress, child,
        [this, child, base_name](qint64 done, qint64 total)
        {
            const int idx = tabs_->indexOf(child);
            if (idx < 0) return;
            const int percent = (total > 0) ? static_cast<int>(done * 100 / total) : 0;
            tabs_->setTabText(idx, QStringLiteral("%1 %2%").arg(base_name).arg(percent));
        });

    connect(task, &BackgroundTask::finished, child,
        [this, child, base_name, on_ready, task](bool ok)
        {
            if (ok)
            {
                const int idx = tabs_->indexOf(child);
                if (idx >= 0) tabs_->setTabText(idx, base_name);
                child->refresh();
                if (on_ready) on_ready(child);
            }
            task->deleteLater();
        });

    task->start();
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

bool LogViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == tabs_->tabBar() && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::MiddleButton)
        {
            const int index = tabs_->tabBar()->tabAt(mouse_event->pos());
            // index 0 is the non-closable "Base" tab
            if (index > 0)
            {
                closeTab(index);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void LogViewer::showTabContextMenu(const QPoint& pos)
{
    QTabBar* bar = tabs_->tabBar();
    const int index = bar->tabAt(pos);
    // index 0 is the non-closable "Base" tab and has no grep associated
    if (index <= 0)
        return;

    QMenu menu;
    QAction* close_action = menu.addAction(Lang::tr("tab.close"));
    QAction* copy_action = menu.addAction(Lang::tr("logviewer.copy_grep"));
    QAction* chosen = menu.exec(bar->mapToGlobal(pos));
    if (chosen == close_action)
    {
        closeTab(index);
    }
    else if (chosen == copy_action)
    {
        GrepNode* child = grep_node_->getChildren()[index - 1];
        QApplication::clipboard()->setText(QString::fromStdString(child->getPattern()));
    }
}

GrepNode* LogViewer::getGrepNode()
{
    return grep_node_;
}

void LogViewer::refresh()
{
    if (text_) text_->refresh();
}
