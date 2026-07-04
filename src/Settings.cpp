#include "Settings.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QSettings>

namespace
{
constexpr auto kEditorGroup = "Editor";
constexpr auto kFontFamilyKey = "FontFamily";
constexpr auto kFontSizeKey = "FontSize";
constexpr auto kHighlightLineColorKey = "HighlightLineColor";
constexpr auto kSearchHighlightColorKey = "SearchHighlightColor";
constexpr auto kSearchCurrentMatchColorKey = "SearchCurrentMatchColor";
constexpr auto kUpdatesGroup = "Updates";
constexpr auto kCheckOnStartupKey = "CheckOnStartup";
constexpr auto kSkippedVersionKey = "SkippedVersion";
constexpr auto kRecentGroup = "Recent";
constexpr auto kRecentFilesKey = "Files";
constexpr auto kRecentProjectsKey = "Projects";
constexpr auto kPathsGroup = "Paths";
constexpr auto kLastFileDirKey = "LastFileDirectory";
constexpr auto kLastProjectDirKey = "LastProjectDirectory";
constexpr auto kGeneralGroup = "General";
constexpr auto kPromptSaveOnExitKey = "PromptSaveOnExit";
constexpr auto kLanguageKey = "Language";
constexpr auto kBookmarksGroup = "Bookmarks";
constexpr auto kDisabledTagsKey = "DisabledTags";
constexpr auto kDisabledFilesKey = "DisabledFiles";

QString normalizePath(const QString& path)
{
    const QString absolute = QFileInfo(path).absoluteFilePath();
    return absolute.isEmpty() ? path : absolute;
}

void prependRecent(QStringList& list, const QString& path)
{
    const QString normalized = normalizePath(path);
    if (normalized.isEmpty())
        return;
    list.removeAll(normalized);
    list.prepend(normalized);
    while (list.size() > Settings::kMaxRecent)
        list.removeLast();
}
} // namespace

Settings::Settings(QObject* parent)
    : QObject(parent), editor_font_(defaultEditorFont())
{
    load();
}

Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}

QString Settings::filePath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("LogView.ini"));
}

QFont Settings::defaultEditorFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    return font;
}

void Settings::setEditorFont(const QFont& font)
{
    if (editor_font_ == font)
        return;
    editor_font_ = font;
    save();
    emit changed();
}

void Settings::setHighlightLineColor(const QColor& color)
{
    if (highlight_line_color_ == color)
        return;
    highlight_line_color_ = color;
    save();
    emit changed();
}

void Settings::setSearchHighlightColor(const QColor& color)
{
    if (search_highlight_color_ == color)
        return;
    search_highlight_color_ = color;
    save();
    emit changed();
}

void Settings::setSearchCurrentMatchColor(const QColor& color)
{
    if (search_current_match_color_ == color)
        return;
    search_current_match_color_ = color;
    save();
    emit changed();
}

void Settings::setCheckForUpdatesOnStartup(bool enabled)
{
    if (check_updates_on_startup_ == enabled)
        return;
    check_updates_on_startup_ = enabled;
    save();
    emit changed();
}

void Settings::setPromptSaveOnExit(bool enabled)
{
    if (prompt_save_on_exit_ == enabled)
        return;
    prompt_save_on_exit_ = enabled;
    save();
    emit changed();
}

void Settings::setLanguage(const QString& code)
{
    if (language_ == code)
        return;
    language_ = code;
    save();
    emit changed();
}

void Settings::setSkippedUpdateVersion(const QString& version)
{
    if (skipped_update_version_ == version)
        return;
    skipped_update_version_ = version;
    save();
    emit changed();
}

void Settings::setLastOpenedFileDirectory(const QString& directory)
{
    if (last_file_dir_ == directory)
        return;
    last_file_dir_ = directory;
    save();
    emit changed();
}

void Settings::setLastOpenedProjectDirectory(const QString& directory)
{
    if (last_project_dir_ == directory)
        return;
    last_project_dir_ = directory;
    save();
    emit changed();
}

void Settings::addRecentFile(const QString& path)
{
    prependRecent(recent_files_, path);
    save();
    emit changed();
}

void Settings::addRecentProject(const QString& path)
{
    prependRecent(recent_projects_, path);
    save();
    emit changed();
}

void Settings::removeRecentFile(const QString& path)
{
    if (recent_files_.removeAll(normalizePath(path)) > 0)
    {
        save();
        emit changed();
    }
}

void Settings::removeRecentProject(const QString& path)
{
    if (recent_projects_.removeAll(normalizePath(path)) > 0)
    {
        save();
        emit changed();
    }
}

void Settings::setDisabledBookmarkTags(const QStringList& tags)
{
    if (disabled_bookmark_tags_ == tags)
        return;
    disabled_bookmark_tags_ = tags;
    save();
    emit changed();
}

void Settings::setDisabledBookmarkFiles(const QStringList& files)
{
    if (disabled_bookmark_files_ == files)
        return;
    disabled_bookmark_files_ = files;
    save();
    emit changed();
}

void Settings::load()
{
    QSettings settings(filePath(), QSettings::IniFormat);
    settings.beginGroup(kEditorGroup);

    const QFont fallback = defaultEditorFont();
    const QString family =
        settings.value(kFontFamilyKey, fallback.family()).toString();
    const int size =
        settings.value(kFontSizeKey, fallback.pointSize()).toInt();

    QFont font = fallback;
    font.setFamily(family);
    if (size > 0)
        font.setPointSize(size);
    editor_font_ = font;

    const QString color = settings.value(kHighlightLineColorKey).toString();
    highlight_line_color_ = color.isEmpty() ? QColor() : QColor(color);

    const QString search_color = settings.value(kSearchHighlightColorKey).toString();
    search_highlight_color_ = search_color.isEmpty() ? QColor() : QColor(search_color);

    const QString search_current_color = settings.value(kSearchCurrentMatchColorKey).toString();
    search_current_match_color_ =
        search_current_color.isEmpty() ? QColor() : QColor(search_current_color);

    settings.endGroup();

    settings.beginGroup(kUpdatesGroup);
    check_updates_on_startup_ =
        settings.value(kCheckOnStartupKey, true).toBool();
    skipped_update_version_ =
        settings.value(kSkippedVersionKey).toString();
    settings.endGroup();

    settings.beginGroup(kGeneralGroup);
    prompt_save_on_exit_ =
        settings.value(kPromptSaveOnExitKey, true).toBool();
    language_ =
        settings.value(kLanguageKey, QStringLiteral("system")).toString();
    settings.endGroup();

    settings.beginGroup(kRecentGroup);
    recent_files_ = settings.value(kRecentFilesKey).toStringList();
    recent_projects_ = settings.value(kRecentProjectsKey).toStringList();
    settings.endGroup();

    settings.beginGroup(kPathsGroup);
    last_file_dir_ = settings.value(kLastFileDirKey).toString();
    last_project_dir_ = settings.value(kLastProjectDirKey).toString();
    settings.endGroup();

    settings.beginGroup(kBookmarksGroup);
    disabled_bookmark_tags_ = settings.value(kDisabledTagsKey).toStringList();
    disabled_bookmark_files_ = settings.value(kDisabledFilesKey).toStringList();
    settings.endGroup();

    while (recent_files_.size() > kMaxRecent)
        recent_files_.removeLast();
    while (recent_projects_.size() > kMaxRecent)
        recent_projects_.removeLast();
}

void Settings::save() const
{
    QSettings settings(filePath(), QSettings::IniFormat);
    settings.beginGroup(kEditorGroup);

    settings.setValue(kFontFamilyKey, editor_font_.family());
    settings.setValue(kFontSizeKey, editor_font_.pointSize());
    settings.setValue(kHighlightLineColorKey,
                      highlight_line_color_.isValid()
                          ? highlight_line_color_.name(QColor::HexArgb)
                          : QString());
    settings.setValue(kSearchHighlightColorKey,
                      search_highlight_color_.isValid()
                          ? search_highlight_color_.name(QColor::HexArgb)
                          : QString());
    settings.setValue(kSearchCurrentMatchColorKey,
                      search_current_match_color_.isValid()
                          ? search_current_match_color_.name(QColor::HexArgb)
                          : QString());

    settings.endGroup();

    settings.beginGroup(kUpdatesGroup);
    settings.setValue(kCheckOnStartupKey, check_updates_on_startup_);
    settings.setValue(kSkippedVersionKey, skipped_update_version_);
    settings.endGroup();

    settings.beginGroup(kGeneralGroup);
    settings.setValue(kPromptSaveOnExitKey, prompt_save_on_exit_);
    settings.setValue(kLanguageKey, language_);
    settings.endGroup();

    settings.beginGroup(kRecentGroup);
    settings.setValue(kRecentFilesKey, recent_files_);
    settings.setValue(kRecentProjectsKey, recent_projects_);
    settings.endGroup();

    settings.beginGroup(kPathsGroup);
    settings.setValue(kLastFileDirKey, last_file_dir_);
    settings.setValue(kLastProjectDirKey, last_project_dir_);
    settings.endGroup();

    settings.beginGroup(kBookmarksGroup);
    settings.setValue(kDisabledTagsKey, disabled_bookmark_tags_);
    settings.setValue(kDisabledFilesKey, disabled_bookmark_files_);
    settings.endGroup();
}
