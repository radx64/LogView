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

    if (role == Qt::DecorationRole)
    {
        constexpr int icon_size = 16;
        return QPixmap(bookmarks_[index.row()].icon_).scaled(icon_size, icon_size);
    }

    return QVariant();
}

void BookmarksModel::add_bookmark(const uint32_t line, const QString& icon, const QString& text)
{
    bookmarks_.append(Bookmark{line, text, icon});
    std::sort(bookmarks_.begin(), bookmarks_.end());

    QModelIndex firstElement = createIndex(0,0);
    QModelIndex lastElement = createIndex(0,0);
    // TODO: Emit real change index for interface update;
    emit dataChanged(firstElement, lastElement, QVector<int>{Qt::DisplayRole});
    // Emit signal for project tracking. Maybe reuse signal dataChanged later;
    emit changed();
}

Bookmark BookmarksModel::get_bookmark(uint32_t index)
{
    if (static_cast<int>(index) < bookmarks_.size()) return bookmarks_[static_cast<int>(index)];
    return Bookmark();
}
