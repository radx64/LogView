#pragma once

#include <QObject>
#include <Qt>

class QString;

// Manages the application color theme (System / Light / Dark).
// The selected mode is persisted in an INI file located next to the binary.
class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum class Theme
    {
        System,
        Light,
        Dark
    };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject* parent = nullptr);

    Theme theme() const { return theme_; }
    void setTheme(Theme theme);

    // Resolves System to the actual active scheme.
    bool isDarkActive() const;

    // Loads persisted setting and applies it.
    void loadAndApply();

    static QString settingsFilePath();

signals:
    void themeChanged();

private:
    void load();
    void save() const;
    void apply();
    Qt::ColorScheme effectiveScheme() const;

    Theme theme_ = Theme::System;
};
