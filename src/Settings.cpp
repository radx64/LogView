#include "Settings.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QSettings>

namespace
{
constexpr auto kEditorGroup = "Editor";
constexpr auto kFontFamilyKey = "FontFamily";
constexpr auto kFontSizeKey = "FontSize";
constexpr auto kHighlightLineColorKey = "HighlightLineColor";
constexpr auto kUpdatesGroup = "Updates";
constexpr auto kCheckOnStartupKey = "CheckOnStartup";
constexpr auto kSkippedVersionKey = "SkippedVersion";
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

void Settings::setCheckForUpdatesOnStartup(bool enabled)
{
    if (check_updates_on_startup_ == enabled)
        return;
    check_updates_on_startup_ = enabled;
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

    settings.endGroup();

    settings.beginGroup(kUpdatesGroup);
    check_updates_on_startup_ =
        settings.value(kCheckOnStartupKey, true).toBool();
    skipped_update_version_ =
        settings.value(kSkippedVersionKey).toString();
    settings.endGroup();
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

    settings.endGroup();

    settings.beginGroup(kUpdatesGroup);
    settings.setValue(kCheckOnStartupKey, check_updates_on_startup_);
    settings.setValue(kSkippedVersionKey, skipped_update_version_);
    settings.endGroup();
}
