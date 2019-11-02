#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <memory>

#include <QFile>
#include <QMessageBox>
#include <QVector>
#include <QObject>

#include "BookmarksModel.hpp"
#include "GrepNode.hpp"

namespace serializer { class Logfile; }

struct Line
{
    uint32_t number;
    QString text;
};

using Lines = QVector<Line>;

class Logfile : public QObject
{
Q_OBJECT
public:
    Logfile(const QString& filename);
    const Lines& getLines() const;
    const QString& getFileName() const;
    BookmarksModel* getBookmarksModel();

    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<BookmarksModel> bookmarks_model_;

protected:
    Lines lines_;
    QString filename_;

    friend class serializer::Logfile;

protected slots:
    void grep_hierarchy_changed();
    void bookmarks_model_changed();

signals:
    void changed();
};

#endif // LOGFILE_HPP
