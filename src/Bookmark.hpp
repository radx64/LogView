#ifndef BOOKMARK_HPP
#define BOOKMARK_HPP

#include <cstdint>
#include <QString>

#include <QAbstractListModel>

struct Bookmark
{
    uint32_t line_number;
    QString bookmark_text;
};

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
#endif // BOOKMARK_HPP
