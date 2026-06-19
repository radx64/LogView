#include "Logfile.hpp"

#include <memory>

#include <QObject>

#include "BookmarksModel.hpp"
#include "MarkingsModel.hpp"
#include "GrepNode.hpp"
#include "FileLineSource.hpp"

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
