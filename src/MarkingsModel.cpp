#include "MarkingsModel.hpp"

#include <QVariant>

MarkingsModel::MarkingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MarkingsModel::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    return markings_.size();
}

QVariant MarkingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= markings_.size()) return QVariant();

    if (role == Qt::DisplayRole)
        return markings_[index.row()].text_;

    if (role == Qt::DecorationRole)
        return marking_color_icon(markings_[index.row()].color_);

    return QVariant();
}

void MarkingsModel::add_marking(const QString& text)
{
    add_marking(text, marking_colors::colorForIndex(markings_.size()));
}

void MarkingsModel::add_marking(const QString& text, const QString& color)
{
    if (text.isEmpty()) return;

    for (const auto& marking : markings_)
    {
        if (marking.text_ == text) return;  // ignore duplicates
    }

    const int row = markings_.size();
    beginInsertRows(QModelIndex(), row, row);
    markings_.append(Marking{text, color});
    endInsertRows();

    emit changed();
}

void MarkingsModel::update_marking(uint32_t index, const QString& text, const QString& color)
{
    if (static_cast<int>(index) >= markings_.size()) return;

    markings_[static_cast<int>(index)].text_ = text;
    markings_[static_cast<int>(index)].color_ = color;

    const QModelIndex changed_index = createIndex(static_cast<int>(index), 0);
    emit dataChanged(changed_index, changed_index, QVector<int>{Qt::DisplayRole, Qt::DecorationRole});
    emit changed();
}

void MarkingsModel::remove_marking(uint32_t index)
{
    if (static_cast<int>(index) >= markings_.size()) return;

    beginRemoveRows(QModelIndex(), static_cast<int>(index), static_cast<int>(index));
    markings_.removeAt(static_cast<int>(index));
    endRemoveRows();

    emit changed();
}

Marking MarkingsModel::get_marking(uint32_t index) const
{
    if (static_cast<int>(index) < markings_.size()) return markings_[static_cast<int>(index)];
    return Marking();
}
