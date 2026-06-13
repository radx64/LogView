#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <memory>

#include <QObject>
#include <QString>

#include "Line.hpp"
#include "BookmarksModel.hpp"
#include "GrepNode.hpp"

namespace serializer { class Logfile; }

class FileLineSource;
class LineSource;

class Logfile : public QObject
{
Q_OBJECT
public:
    Logfile(const QString& filename);
    const QString& getFileName() const;
    BookmarksModel* getBookmarksModel();

    std::shared_ptr<LineSource> getLineSource() const;

    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<BookmarksModel> bookmarks_model_;

protected:
    std::shared_ptr<FileLineSource> source_;
    QString filename_;

    void connect_events();

    friend class serializer::Logfile;

protected slots:
    void grep_hierarchy_changed();
    void bookmarks_model_changed();

signals:
    void changed();
};

#endif // LOGFILE_HPP
