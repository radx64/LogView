#include "ThemeManager.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QPalette>
#include <QSettings>
#include <QStyle>
#include <QStyleHints>

namespace
{
constexpr auto kGroup = "Appearance";
constexpr auto kThemeKey = "Theme";

QString themeToString(ThemeManager::Theme theme)
{
    switch (theme)
    {
        case ThemeManager::Theme::Light: return QStringLiteral("Light");
        case ThemeManager::Theme::Dark:  return QStringLiteral("Dark");
        case ThemeManager::Theme::System:
        default:                         return QStringLiteral("System");
    }
}

ThemeManager::Theme themeFromString(const QString& value)
{
    if (value.compare(QStringLiteral("Light"), Qt::CaseInsensitive) == 0)
        return ThemeManager::Theme::Light;
    if (value.compare(QStringLiteral("Dark"), Qt::CaseInsensitive) == 0)
        return ThemeManager::Theme::Dark;
    return ThemeManager::Theme::System;
}

QPalette buildPalette(bool dark)
{
    QPalette palette;
    if (dark)
    {
        const QColor window(53, 53, 53);
        const QColor base(35, 35, 35);
        const QColor alternateBase(45, 45, 45);
        const QColor text(220, 220, 220);
        const QColor disabledText(127, 127, 127);
        const QColor highlight(42, 130, 218);

        palette.setColor(QPalette::Window, window);
        palette.setColor(QPalette::WindowText, text);
        palette.setColor(QPalette::Base, base);
        palette.setColor(QPalette::AlternateBase, alternateBase);
        palette.setColor(QPalette::ToolTipBase, window);
        palette.setColor(QPalette::ToolTipText, text);
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::Button, window);
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Link, highlight);
        palette.setColor(QPalette::Highlight, highlight);
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::PlaceholderText, disabledText);

        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    }
    else
    {
        palette = QApplication::style()->standardPalette();
    }
    return palette;
}
} // namespace

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this](Qt::ColorScheme)
            {
                if (theme_ == Theme::System)
                {
                    apply();
                    emit themeChanged();
                }
            });
}

QString ThemeManager::settingsFilePath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("LogView.ini"));
}

Qt::ColorScheme ThemeManager::effectiveScheme() const
{
    switch (theme_)
    {
        case Theme::Light: return Qt::ColorScheme::Light;
        case Theme::Dark:  return Qt::ColorScheme::Dark;
        case Theme::System:
        default:           return QGuiApplication::styleHints()->colorScheme();
    }
}

bool ThemeManager::isDarkActive() const
{
    return effectiveScheme() == Qt::ColorScheme::Dark;
}

void ThemeManager::setTheme(Theme theme)
{
    if (theme_ == theme)
        return;
    theme_ = theme;
    apply();
    save();
    emit themeChanged();
}

void ThemeManager::loadAndApply()
{
    load();
    apply();
    emit themeChanged();
}

void ThemeManager::load()
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    const QString value =
        settings.value(QStringLiteral("%1/%2").arg(kGroup, kThemeKey),
                       QStringLiteral("System")).toString();
    theme_ = themeFromString(value);
}

void ThemeManager::save() const
{
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.setValue(QStringLiteral("%1/%2").arg(kGroup, kThemeKey),
                      themeToString(theme_));
}

void ThemeManager::apply()
{
    QApplication::setStyle(QStringLiteral("Fusion"));
    QApplication::setPalette(buildPalette(isDarkActive()));
}
