#include "BookmarksModel.hpp"

#include <algorithm>
#include <QDebug>
#include <QPixmap>
#include <QRegularExpression>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

#include "AutoBookmark.hpp"
#include "LineSource.hpp"

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

bool BookmarksModel::compute_auto_bookmarks(
    const LineSource& source,
    const QVector<AutoBookmark>& rules,
    QVector<Bookmark>& out,
    const std::function<bool(qint64 done, qint64 total)>& report)
{
    out.clear();

    if (rules.isEmpty())
    {
        report(1, 1);
        return true;
    }

    QVector<QRegularExpression> expressions;
    expressions.reserve(rules.size());
    for (const AutoBookmark& rule : rules)
    {
        const QRegularExpression expression = rule.regularExpression();
        expressions.append(expression.isValid() ? expression : QRegularExpression());
    }

    const qint64 line_count = source.count();
    for (qint64 row = 0; row < line_count; ++row)
    {
        const Line line = source.at(row);
        for (int i = 0; i < rules.size(); ++i)
        {
            if (rules[i].pattern_.isEmpty() || !expressions[i].isValid())
                continue;
            if (expressions[i].match(line.text).hasMatch())
            {
                Bookmark bookmark{line.number, rules[i].name_, rules[i].icon_};
                bookmark.is_auto_ = true;
                bookmark.tags_ = rules[i].tags_;
                out.append(bookmark);
            }
        }

        // Report progress every 1024 lines to avoid excessive calls to the report function.
        if ((row & 1023) == 0)
        {
            if (!report(row, line_count))
                return false;
        }
    }

    report(line_count, line_count);
    return true;
}

void BookmarksModel::apply_auto_bookmarks(const QVector<Bookmark>& auto_bookmarks)
{
    beginResetModel();

    bookmarks_.erase(
        std::remove_if(bookmarks_.begin(), bookmarks_.end(),
                       [](const Bookmark& b) { return b.is_auto_; }),
        bookmarks_.end());

    for (const Bookmark& bookmark : auto_bookmarks)
    {
        const auto position = std::upper_bound(
            bookmarks_.begin(), bookmarks_.end(), bookmark);
        bookmarks_.insert(position, bookmark);
    }

    endResetModel();
    emit changed();
}
