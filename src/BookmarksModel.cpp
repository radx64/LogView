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
    const Bookmark bookmark{line, text, icon};
    const auto position = std::upper_bound(bookmarks_.begin(), bookmarks_.end(), bookmark);
    const int row = static_cast<int>(std::distance(bookmarks_.begin(), position));

    beginInsertRows(QModelIndex(), row, row);
    bookmarks_.insert(position, bookmark);
    endInsertRows();

    emit changed();
}

void BookmarksModel::update_bookmark(uint32_t index, const QString& icon, const QString& text)
{
    if (static_cast<int>(index) >= bookmarks_.size()) return;

    bookmarks_[static_cast<int>(index)].text_ = text;
    bookmarks_[static_cast<int>(index)].icon_ = icon;

    const QModelIndex changed_index = createIndex(static_cast<int>(index), 0);
    emit dataChanged(changed_index, changed_index, QVector<int>{Qt::DisplayRole, Qt::DecorationRole});
    emit changed();
}

void BookmarksModel::remove_bookmark(uint32_t index)
{
    if (static_cast<int>(index) >= bookmarks_.size()) return;

    beginRemoveRows(QModelIndex(), static_cast<int>(index), static_cast<int>(index));
    bookmarks_.removeAt(static_cast<int>(index));
    endRemoveRows();

    emit changed();
}

Bookmark BookmarksModel::get_bookmark(uint32_t index)
{
    if (static_cast<int>(index) < bookmarks_.size()) return bookmarks_[static_cast<int>(index)];
    return Bookmark();
}
