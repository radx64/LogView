#include "Bookmark.hpp"

BookmarksModel::BookmarksModel(QObject *parent){};

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    return bookmarks_.size();
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
       return QString(QString().number(bookmarks_[index.row()].line_number)
               + " : "+ bookmarks_[index.row()].bookmark_text);
    return QVariant();
}

void BookmarksModel::add_bookmark(const Bookmark& bookmark)
{
    bookmarks_.append(bookmark);
    QModelIndex firstElement = createIndex(0,0);
    QModelIndex lastElement = createIndex(0,0);
    emit dataChanged(firstElement, lastElement, {Qt::DisplayRole});
}
