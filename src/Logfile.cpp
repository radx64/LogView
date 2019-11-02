#include "Logfile.hpp"
#include <memory>

#include <QFile>
#include <QMessageBox>
#include <QVector>
#include <QObject>

#include "BookmarksModel.hpp"
#include "GrepNode.hpp"

Logfile::Logfile(const QString& filename)
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
    connect_events();
}

void Logfile::connect_events()
{
    QObject::connect(bookmarks_model_.get(), &BookmarksModel::changed,
                     this, &Logfile::bookmarks_model_changed);

    QObject::connect(grep_hierarchy_.get(), &GrepNode::changed,
                     this, &Logfile::grep_hierarchy_changed);
}

const Lines& Logfile::getLines() const
{
    return lines_;
}
const QString& Logfile::getFileName() const
{
    return filename_;
}

BookmarksModel* Logfile::getBookmarksModel()
{
    return bookmarks_model_.get();
}

void Logfile::grep_hierarchy_changed()
{
    emit changed();
}
void Logfile::bookmarks_model_changed()
{
    emit changed();
}
