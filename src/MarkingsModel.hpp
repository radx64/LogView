#pragma once

#include <QString>
#include <QVector>
#include <QAbstractListModel>

#include "Marking.hpp"

namespace serializer { class MarkingsModel; }

class MarkingsModel : public QAbstractListModel
{
Q_OBJECT
public:
    MarkingsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Adds a marking, auto-assigning a color from the palette. Duplicate text is ignored.
    void add_marking(const QString& text);
    void add_marking(const QString& text, const QString& color);
    void update_marking(uint32_t index, const QString& text, const QString& color);
    void remove_marking(uint32_t index);
    Marking get_marking(uint32_t index) const;

    const QVector<Marking>& markings() const { return markings_; }

protected:
    QVector<Marking> markings_;

    friend class serializer::MarkingsModel;

signals:
    void changed();
};
