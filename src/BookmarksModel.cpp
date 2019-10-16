#include "BookmarksModel.hpp"

#include <algorithm>
#include <QDebug>
#include <QPixmap>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

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
       return QString(bookmarks_[index.row()].text_);
    else if (role == Qt::DecorationRole)
        return QPixmap(bookmarks_[index.row()].icon_);
    return QVariant();
}

void BookmarksModel::add_bookmark(const uint32_t line, const QString& icon, const QString& text)
{
    bookmarks_.append(Bookmark{line, text, icon});
    std::sort(bookmarks_.begin(), bookmarks_.end());

    QModelIndex firstElement = createIndex(0,0);
    QModelIndex lastElement = createIndex(0,0);
    // TODO: Emit real change index
    emit dataChanged(firstElement, lastElement, QVector<int>{Qt::DisplayRole});
}

Bookmark BookmarksModel::get_bookmark(uint32_t index)
{
    if (static_cast<int>(index) < bookmarks_.size()) return bookmarks_[static_cast<int>(index)];
    return Bookmark();
}

void BookmarksModel::serialize(QJsonObject &json) const
{
    QJsonArray array;
    for (const auto& bookmark : bookmarks_)
    {
        QJsonObject jsonBookmark;
        bookmark.serialize(jsonBookmark);
        array.append(jsonBookmark);
    }
    json["bookmarks"] = array;
}
void BookmarksModel::deserialize(const QJsonObject &json)
{
    (void)json;
}
