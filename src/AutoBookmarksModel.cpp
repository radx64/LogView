#include "AutoBookmarksModel.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Settings.hpp"

namespace
{
constexpr auto kRulesKey = "autobookmarks";
constexpr auto kNameKey = "name";
constexpr auto kDescriptionKey = "description";
constexpr auto kPatternKey = "pattern";
constexpr auto kIconKey = "icon";
constexpr auto kTagsKey = "tags";
constexpr auto kEnabledKey = "enabled";
constexpr auto kCaseSensitiveKey = "case_sensitive";

constexpr auto kDefaultIcon = ":/icon/Gnome-Bookmark-New-32.png";

QJsonObject toJson(const AutoBookmark& rule)
{
    QJsonObject object;
    object[kNameKey] = rule.name_;
    object[kDescriptionKey] = rule.description_;
    object[kPatternKey] = rule.pattern_;
    object[kIconKey] = rule.icon_;
    object[kTagsKey] = QJsonArray::fromStringList(rule.tags_);
    object[kEnabledKey] = rule.enabled_;
    object[kCaseSensitiveKey] = rule.case_sensitive_;
    return object;
}

AutoBookmark fromJson(const QJsonObject& object)
{
    AutoBookmark rule;
    rule.name_ = object[kNameKey].toString();
    rule.description_ = object[kDescriptionKey].toString();
    rule.pattern_ = object[kPatternKey].toString();
    rule.icon_ = object[kIconKey].toString();
    if (rule.icon_.isEmpty())
        rule.icon_ = QString::fromLatin1(kDefaultIcon);
    QStringList tags;
    for (const QJsonValue& tag : object[kTagsKey].toArray())
        tags.append(tag.toString());
    rule.tags_ = tags;
    rule.enabled_ = object[kEnabledKey].toBool(true);
    rule.case_sensitive_ = object[kCaseSensitiveKey].toBool(true);
    return rule;
}
} // namespace

AutoBookmarksModel::AutoBookmarksModel(QObject* parent)
    : QObject(parent)
{
    load();
}

AutoBookmarksModel& AutoBookmarksModel::instance()
{
    static AutoBookmarksModel model;
    return model;
}

QString AutoBookmarksModel::dirPath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("bookmarks"));
}

void AutoBookmarksModel::load()
{
    files_.clear();

    QDir dir(dirPath());
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    ensureExampleFile();

    const QStringList entries =
        dir.entryList(QStringList{QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QString& entry : entries)
    {
        QFile file(dir.filePath(entry));
        if (!file.open(QIODevice::ReadOnly))
            continue;

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        const QJsonArray rules = document.object()[kRulesKey].toArray();

        QVector<AutoBookmark> parsed;
        for (const QJsonValue& value : rules)
        {
            AutoBookmark rule = fromJson(value.toObject());
            if (!rule.pattern_.isEmpty())
                parsed.append(rule);
        }
        files_.insert(entry, parsed);
    }

    loadFilterState();

    emit filesChanged();
    emit changed();
}

void AutoBookmarksModel::ensureExampleFile()
{
    QDir dir(dirPath());
    const QStringList entries =
        dir.entryList(QStringList{QStringLiteral("*.json")}, QDir::Files);
    if (!entries.isEmpty())
        return;

    const QString example = dir.filePath(QStringLiteral("example.json"));
    QFile file(example);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QJsonArray rules;
    rules.append(toJson(AutoBookmark(
        QStringLiteral("Errors"),
        QStringLiteral("Bookmark every line that contains the word ERROR."),
        QStringLiteral("\\bERROR\\b"),
        QString::fromLatin1(kDefaultIcon),
        QStringList{QStringLiteral("severity")},
        true, true)));
    rules.append(toJson(AutoBookmark(
        QStringLiteral("Warnings"),
        QStringLiteral("Bookmark every line that contains the word WARNING."),
        QStringLiteral("\\bWARNING\\b"),
        QStringLiteral(":/icon/Gnome-Dialog-Warning-32.png"),
        QStringList{QStringLiteral("severity")},
        true, true)));

    QJsonObject root;
    root[kRulesKey] = rules;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void AutoBookmarksModel::loadFilterState()
{
    disabled_tags_.clear();
    for (const QString& tag : Settings::instance().disabledBookmarkTags())
        disabled_tags_.insert(tag);

    disabled_files_.clear();
    for (const QString& file : Settings::instance().disabledBookmarkFiles())
        disabled_files_.insert(file);
}

QStringList AutoBookmarksModel::files() const
{
    return QStringList(files_.keys());
}

QVector<AutoBookmark> AutoBookmarksModel::rules(const QString& file) const
{
    return files_.value(file);
}

AutoBookmark AutoBookmarksModel::rule(const QString& file, int row) const
{
    const QVector<AutoBookmark> list = files_.value(file);
    if (row < 0 || row >= list.size())
        return AutoBookmark();
    return list[row];
}

void AutoBookmarksModel::addRule(const QString& file, const AutoBookmark& rule)
{
    if (!files_.contains(file) || rule.pattern_.isEmpty())
        return;
    files_[file].append(rule);
    saveFile(file);
    emit changed();
}

void AutoBookmarksModel::updateRule(const QString& file, int row, const AutoBookmark& rule)
{
    if (!files_.contains(file) || rule.pattern_.isEmpty())
        return;
    QVector<AutoBookmark>& list = files_[file];
    if (row < 0 || row >= list.size())
        return;
    list[row] = rule;
    saveFile(file);
    emit changed();
}

void AutoBookmarksModel::removeRule(const QString& file, int row)
{
    if (!files_.contains(file))
        return;
    QVector<AutoBookmark>& list = files_[file];
    if (row < 0 || row >= list.size())
        return;
    list.removeAt(row);
    saveFile(file);
    emit changed();
}

QString AutoBookmarksModel::createFile(const QString& name)
{
    QString base = name.trimmed();
    if (base.isEmpty())
        return QString();
    if (!base.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive))
        base += QStringLiteral(".json");
    if (files_.contains(base))
        return QString();

    files_.insert(base, QVector<AutoBookmark>());
    saveFile(base);
    emit filesChanged();
    emit changed();
    return base;
}

void AutoBookmarksModel::deleteFile(const QString& file)
{
    if (!files_.contains(file))
        return;
    files_.remove(file);
    QFile::remove(QDir(dirPath()).filePath(file));
    emit filesChanged();
    emit changed();
}

QStringList AutoBookmarksModel::allTags() const
{
    QStringList tags;
    for (const QVector<AutoBookmark>& list : files_)
        for (const AutoBookmark& rule : list)
            for (const QString& tag : rule.tags_)
                if (!tag.isEmpty() && !tags.contains(tag))
                    tags.append(tag);
    tags.sort(Qt::CaseInsensitive);
    return tags;
}

bool AutoBookmarksModel::isTagEnabled(const QString& tag) const
{
    return !disabled_tags_.contains(tag);
}

bool AutoBookmarksModel::isFileEnabled(const QString& file) const
{
    return !disabled_files_.contains(file);
}

void AutoBookmarksModel::setTagEnabled(const QString& tag, bool enabled)
{
    const bool currently = isTagEnabled(tag);
    if (currently == enabled)
        return;
    if (enabled)
        disabled_tags_.remove(tag);
    else
        disabled_tags_.insert(tag);
    Settings::instance().setDisabledBookmarkTags(QStringList(disabled_tags_.values()));
    emit changed();
}

void AutoBookmarksModel::setFileEnabled(const QString& file, bool enabled)
{
    const bool currently = isFileEnabled(file);
    if (currently == enabled)
        return;
    if (enabled)
        disabled_files_.remove(file);
    else
        disabled_files_.insert(file);
    Settings::instance().setDisabledBookmarkFiles(QStringList(disabled_files_.values()));
    emit changed();
}

QVector<AutoBookmark> AutoBookmarksModel::enabledRules() const
{
    QVector<AutoBookmark> result;
    for (auto it = files_.constBegin(); it != files_.constEnd(); ++it)
    {
        if (disabled_files_.contains(it.key()))
            continue;

        for (const AutoBookmark& rule : it.value())
        {
            if (!rule.enabled_ || rule.pattern_.isEmpty())
                continue;

            bool tag_disabled = false;
            for (const QString& tag : rule.tags_)
            {
                if (disabled_tags_.contains(tag))
                {
                    tag_disabled = true;
                    break;
                }
            }
            if (tag_disabled)
                continue;

            result.append(rule);
        }
    }
    return result;
}

void AutoBookmarksModel::saveFile(const QString& file) const
{
    if (!files_.contains(file))
        return;

    QJsonArray rules;
    for (const AutoBookmark& rule : files_.value(file))
        rules.append(toJson(rule));

    QJsonObject root;
    root[kRulesKey] = rules;

    QDir dir(dirPath());
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QFile out(dir.filePath(file));
    if (!out.open(QIODevice::WriteOnly))
        return;
    out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}
