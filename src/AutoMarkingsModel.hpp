#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "AutoMarking.hpp"

class AutoMarkingsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        EnabledColumn = 0,
        PatternColumn,
        TextColorColumn,
        BackgroundColorColumn,
        CaseSensitiveColumn,
        ColumnCount
    };

    static AutoMarkingsModel& instance();
    static QString filePath();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

    const QVector<AutoMarking>& markings() const { return markings_; }
    AutoMarking get_marking(int row) const;
    void add_marking(const AutoMarking& marking);
    void update_marking(int row, const AutoMarking& marking);
    void remove_marking(int row);

    void load();
    void save() const;

signals:
    void changed();

private:
    explicit AutoMarkingsModel(QObject* parent = nullptr);
    static QVector<AutoMarking> defaultMarkings();

    QVector<AutoMarking> markings_{};
};
