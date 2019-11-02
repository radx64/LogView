#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <memory>

#include <QFile>
#include <QMessageBox>
#include <QVector>

#include "BookmarksModel.hpp"
#include "GrepNode.hpp"

namespace serializer { class Logfile; }

struct Line
{
    uint32_t number;
    QString text;
};

using Lines = QVector<Line>;

class Logfile
{
public:
    Logfile(const QString& filename)
    {
        filename_ = filename;
        lines_.clear();

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msg;
            msg.setText("Unable to open file" + file.errorString());
            msg.setStandardButtons(QMessageBox::Ok);
            msg.setIcon(QMessageBox::Critical);
            msg.exec();
            return;
        }

        uint32_t index{};
        while(!file.atEnd())
        {
            ++index;
            lines_.append({index, QString(file.readLine()).trimmed()});
        }

        grep_hierarchy_ = std::make_unique<GrepNode>("ROOT");
        bookmarks_model_ = std::make_unique<BookmarksModel>(nullptr);
    }

    const Lines& getLines() const
    {
        return lines_;
    }
    const QString& getFileName() const
    {
        return filename_;
    }

    BookmarksModel* getBookmarksModel()
    {
        return bookmarks_model_.get();
    }

    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<BookmarksModel> bookmarks_model_;
protected:
    Lines lines_;
    QString filename_;

    friend class serializer::Logfile;
};

#endif // LOGFILE_HPP
