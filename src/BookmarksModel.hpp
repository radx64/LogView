#ifndef BOOKMARKS_MODEL_HPP
#define BOOKMARKS_MODEL_HPP

#include <QAbstractListModel>
#include <QString>

#include "Bookmark.hpp"

class BookmarksModel : public QAbstractListModel
{
    Q_OBJECT
public:
    BookmarksModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void add_bookmark(const Bookmark& bookmark);

protected:
    QVector<Bookmark> bookmarks_;
};
#endif // BOOKMARKS_MODEL_HPP
