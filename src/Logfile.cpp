#include "Logfile.hpp"

#include <memory>

#include <QObject>
#include <QVector>

#include "AutoBookmark.hpp"
#include "AutoBookmarksModel.hpp"
#include "Bookmark.hpp"
#include "BookmarksModel.hpp"
#include "MarkingsModel.hpp"
#include "GrepNode.hpp"
#include "FileLineSource.hpp"
#include "loader/BackgroundTask.hpp"

Logfile::Logfile(const QString& filename)
{
    filename_ = filename;

    source_ = std::make_shared<FileLineSource>(filename);

    grep_hierarchy_ = std::make_unique<GrepNode>("ROOT");
    bookmarks_model_ = std::make_unique<BookmarksModel>(nullptr);
    markings_model_ = std::make_unique<MarkingsModel>(nullptr);
    connect_events();
}

void Logfile::connect_events()
{
    QObject::connect(bookmarks_model_.get(), &BookmarksModel::changed,
                     this, &Logfile::changed);

    QObject::connect(markings_model_.get(), &MarkingsModel::changed,
                     this, &Logfile::changed);

    QObject::connect(grep_hierarchy_.get(), &GrepNode::changed,
                     this, &Logfile::changed);
}

void Logfile::reload()
{
    source_ = std::make_shared<FileLineSource>(filename_);
}

void Logfile::generateAutoBookmarks()
{
    if (!source_)
        return;

    if (auto_bookmark_task_)
    {
        auto_bookmark_task_->cancel();
        auto_bookmark_task_ = nullptr;
    }

    const QVector<AutoBookmark> rules = AutoBookmarksModel::instance().enabledRules();
    std::shared_ptr<LineSource> source = source_;
    auto result = std::make_shared<QVector<Bookmark>>();

    BackgroundTask* task = new BackgroundTask(
        [source, rules, result](const BackgroundTask::ProgressFn& report) -> bool
        {
            return BookmarksModel::compute_auto_bookmarks(*source, rules, *result, report);
        },
        this);
    auto_bookmark_task_ = task;

    connect(task, &BackgroundTask::finished, this,
        [this, task, result](bool ok)
        {
            if (auto_bookmark_task_ == task)
            {
                if (ok)
                    bookmarks_model_->apply_auto_bookmarks(*result);
                auto_bookmark_task_ = nullptr;
                emit autoBookmarksFinished();
            }
            task->deleteLater();
        });

    emit autoBookmarksStarted();
    task->start();
}

std::shared_ptr<LineSource> Logfile::getLineSource() const
{
    return source_;
}

std::shared_ptr<FileLineSource> Logfile::getFileSource() const
{
    return source_;
}

const QString& Logfile::getFileName() const
{
    return filename_;
}

BookmarksModel* Logfile::getBookmarksModel()
{
    return bookmarks_model_.get();
}

MarkingsModel* Logfile::getMarkingsModel()
{
    return markings_model_.get();
}
