#pragma once

#include <memory>

#include <QObject>
#include <QString>

#include "Line.hpp"
#include "BookmarksModel.hpp"
#include "MarkingsModel.hpp"
#include "GrepNode.hpp"

namespace serializer { class Logfile; }

class FileLineSource;
class LineSource;
class BackgroundTask;

class Logfile : public QObject
{
Q_OBJECT
public:
    Logfile(const QString& filename);
    const QString& getFileName() const;
    BookmarksModel* getBookmarksModel();
    MarkingsModel* getMarkingsModel();
    void reload();
    void generateAutoBookmarks();

    std::shared_ptr<LineSource> getLineSource() const;
    std::shared_ptr<FileLineSource> getFileSource() const;

    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<BookmarksModel> bookmarks_model_;
    std::unique_ptr<MarkingsModel> markings_model_;

protected:
    std::shared_ptr<FileLineSource> source_;
    QString filename_;
    BackgroundTask* auto_bookmark_task_{nullptr};

    void connect_events();

    friend class serializer::Logfile;

signals:
    void changed();
    void autoBookmarksStarted();
    void autoBookmarksFinished();
};
