#include "BookmarksModel.hpp"

#include <algorithm>
#include <QDebug>
#include <QPixmap>
#include <QVector>

BookmarksModel::BookmarksModel(QObject *parent)
{
     (void) parent;
};

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    return bookmarks_.size();
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
       return QString(QString().number(bookmarks_[index.row()].line_number)
               + " : "+ bookmarks_[index.row()].bookmark_text);
    else if (role == Qt::DecorationRole)
    {
        return bookmarks_[index.row()].icon;
    }

    return QVariant();
}

void BookmarksModel::add_bookmark(const uint32_t& line, const QPixmap& icon, const QString& text)
{
    bookmarks_.append(Bookmark{line, text, icon});
    std::sort(bookmarks_.begin(), bookmarks_.end());

    QModelIndex firstElement = createIndex(0,0);
    QModelIndex lastElement = createIndex(0,0);
    // TODO: Emit real change index
    emit dataChanged(firstElement, lastElement, QVector<int>{Qt::DisplayRole});
}
