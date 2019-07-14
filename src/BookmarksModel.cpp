#include "BookmarksModel.hpp"

#include <QDebug>
#include <QPixmap>

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
        return QPixmap(":/icon/Dialog-Apply.png");;
    }

    return QVariant();
}

void BookmarksModel::add_bookmark(const uint32_t& line, const QPixmap& icon, const QString& text)
{
    bookmarks_.append(Bookmark{line, text, icon});
    QModelIndex firstElement = createIndex(0,0);
    QModelIndex lastElement = createIndex(0,0);
    emit dataChanged(firstElement, lastElement, {Qt::DisplayRole});
}
