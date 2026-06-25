#pragma once

#include <functional>

#include <QString>
#include <QVector>
#include <QAbstractListModel>

#include "Bookmark.hpp"
class QJsonObject;
class LineSource;
class AutoBookmark;

namespace serializer { class BookmarksModel; }

class BookmarksModel : public QAbstractListModel
{
Q_OBJECT
public:
    BookmarksModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void add_bookmark(const uint32_t line, const QString& icon, const QString& text);
    void update_bookmark(uint32_t index, const QString& icon, const QString& text);
    void remove_bookmark(uint32_t index);
    Bookmark get_bookmark(uint32_t index);

    static bool compute_auto_bookmarks(
        const LineSource& source,
        const QVector<AutoBookmark>& rules,
        QVector<Bookmark>& out,
        const std::function<bool(qint64 done, qint64 total)>& report);

    void apply_auto_bookmarks(const QVector<Bookmark>& auto_bookmarks);


protected:
    QVector<Bookmark> bookmarks_;

    friend class serializer::BookmarksModel;

signals:
    void changed();
};