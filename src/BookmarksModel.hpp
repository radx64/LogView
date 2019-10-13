#ifndef BOOKMARKS_MODEL_HPP
#define BOOKMARKS_MODEL_HPP

#include <QAbstractListModel>
#include <QString>

#include "Bookmark.hpp"

#include "Serializable.hpp"

class QJsonObject;

class BookmarksModel : public QAbstractListModel, Serializable
{
    Q_OBJECT
public:
    BookmarksModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void add_bookmark(const uint32_t line, const QPixmap& icon, const QString& text);
    Bookmark get_bookmark(uint32_t index);

    virtual void serialize(QJsonObject &json) const override;
    virtual void deserialize(const QJsonObject &json) override;

protected:
    QVector<Bookmark> bookmarks_;
};
#endif // BOOKMARKS_MODEL_HPP
