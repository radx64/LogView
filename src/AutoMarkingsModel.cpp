#include "AutoMarkingsModel.hpp"

#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Marking.hpp"
#include "Translator.hpp"

namespace
{
constexpr auto kRulesKey = "automarkings";
constexpr auto kPatternKey = "pattern";
constexpr auto kTextColorKey = "text_color";
constexpr auto kBackgroundColorKey = "background_color";
constexpr auto kEnabledKey = "enabled";
constexpr auto kCaseSensitiveKey = "case_sensitive";

QJsonObject toJson(const AutoMarking& marking)
{
    QJsonObject object;
    object[kPatternKey] = marking.pattern_;
    object[kTextColorKey] = marking.text_color_;
    object[kBackgroundColorKey] = marking.background_color_;
    object[kEnabledKey] = marking.enabled_;
    object[kCaseSensitiveKey] = marking.case_sensitive_;
    return object;
}

AutoMarking fromJson(const QJsonObject& object)
{
    AutoMarking marking;
    marking.pattern_ = object[kPatternKey].toString();
    marking.text_color_ = object[kTextColorKey].toString();
    marking.background_color_ = object[kBackgroundColorKey].toString();
    marking.enabled_ = object[kEnabledKey].toBool(true);
    marking.case_sensitive_ = object[kCaseSensitiveKey].toBool(true);
    return marking;
}
} // namespace

AutoMarkingsModel::AutoMarkingsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    load();
}

AutoMarkingsModel& AutoMarkingsModel::instance()
{
    static AutoMarkingsModel model;
    return model;
}

QString AutoMarkingsModel::filePath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("automarkings.json"));
}

int AutoMarkingsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return markings_.size();
}

int AutoMarkingsModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant AutoMarkingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= markings_.size())
        return QVariant();

    const AutoMarking& marking = markings_[index.row()];

    if (role == Qt::CheckStateRole)
    {
        if (index.column() == EnabledColumn)
            return marking.enabled_ ? Qt::Checked : Qt::Unchecked;
        if (index.column() == CaseSensitiveColumn)
            return marking.case_sensitive_ ? Qt::Checked : Qt::Unchecked;
    }

    if (role == Qt::DecorationRole)
    {
        if (index.column() == TextColorColumn)
            return marking_color_icon(marking.text_color_);
        if (index.column() == BackgroundColorColumn &&
            QColor(marking.background_color_).isValid())
            return marking_color_icon(marking.background_color_);
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case EnabledColumn:
                return marking.enabled_ ? Lang::tr("common.yes") : Lang::tr("common.no");
            case PatternColumn:
                return marking.pattern_;
            case TextColorColumn:
                return marking.text_color_;
            case BackgroundColorColumn:
                return marking.background_color_.isEmpty()
                           ? Lang::tr("common.none_paren")
                           : marking.background_color_;
            case CaseSensitiveColumn:
                return marking.case_sensitive_ ? Lang::tr("common.yes") : Lang::tr("common.no");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant AutoMarkingsModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
        case EnabledColumn:
            return Lang::tr("common.enabled");
        case PatternColumn:
            return Lang::tr("common.regex");
        case TextColorColumn:
            return Lang::tr("common.font_color");
        case BackgroundColorColumn:
            return Lang::tr("common.background");
        case CaseSensitiveColumn:
            return Lang::tr("common.case_sensitive");
        default:
            return QVariant();
    }
}

Qt::ItemFlags AutoMarkingsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags item_flags = QAbstractTableModel::flags(index);
    if (!index.isValid()) return item_flags;
    if (index.column() == EnabledColumn || index.column() == CaseSensitiveColumn)
        item_flags |= Qt::ItemIsUserCheckable;
    return item_flags;
}

bool AutoMarkingsModel::setData(const QModelIndex& index, const QVariant& value,
                                int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= markings_.size())
        return false;
    if (role != Qt::CheckStateRole)
        return false;

    AutoMarking& marking = markings_[index.row()];
    const bool checked = value.toInt() == Qt::Checked;
    if (index.column() == EnabledColumn)
        marking.enabled_ = checked;
    else if (index.column() == CaseSensitiveColumn)
        marking.case_sensitive_ = checked;
    else
        return false;

    emit dataChanged(index, index, QVector<int>{role, Qt::DisplayRole});
    save();
    emit changed();
    return true;
}

AutoMarking AutoMarkingsModel::get_marking(int row) const
{
    if (row < 0 || row >= markings_.size()) return AutoMarking();
    return markings_[row];
}

void AutoMarkingsModel::add_marking(const AutoMarking& marking)
{
    if (marking.pattern_.isEmpty()) return;
    const int row = markings_.size();
    beginInsertRows(QModelIndex(), row, row);
    markings_.append(marking);
    endInsertRows();
    save();
    emit changed();
}

void AutoMarkingsModel::update_marking(int row, const AutoMarking& marking)
{
    if (row < 0 || row >= markings_.size() || marking.pattern_.isEmpty()) return;
    markings_[row] = marking;
    const QModelIndex first = index(row, 0);
    const QModelIndex last = index(row, ColumnCount - 1);
    emit dataChanged(first, last);
    save();
    emit changed();
}

void AutoMarkingsModel::remove_marking(int row)
{
    if (row < 0 || row >= markings_.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    markings_.removeAt(row);
    endRemoveRows();
    save();
    emit changed();
}

void AutoMarkingsModel::load()
{
    beginResetModel();
    markings_.clear();

    QFile file(filePath());
    if (file.open(QIODevice::ReadOnly))
    {
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        const QJsonArray rules = document.object()[kRulesKey].toArray();
        for (const QJsonValue& value : rules)
        {
            AutoMarking marking = fromJson(value.toObject());
            if (!marking.pattern_.isEmpty())
                markings_.append(marking);
        }
    }

    if (markings_.isEmpty())
    {
        markings_ = defaultMarkings();
        save();
    }

    endResetModel();
    emit changed();
}

void AutoMarkingsModel::save() const
{
    QJsonArray rules;
    for (const AutoMarking& marking : markings_)
        rules.append(toJson(marking));

    QJsonObject root;
    root[kRulesKey] = rules;

    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QVector<AutoMarking> AutoMarkingsModel::defaultMarkings()
{
    return {
        AutoMarking(QStringLiteral("WRN/.*"), QStringLiteral("#1565c0")),
        AutoMarking(QStringLiteral("\\bDEBUG\\b"), QStringLiteral("#1565c0")),
        AutoMarking(QStringLiteral("DBG/.*"), QStringLiteral("#9e9e9e")),
        AutoMarking(QStringLiteral("\\bERROR\\b"), QStringLiteral("#c62828")),
        AutoMarking(QStringLiteral("ERR/.*"), QStringLiteral("#c62828")),
    };
}
