#pragma once

#include <QObject>
#include <Qt>

class QString;

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

    bool isDarkActive() const;

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
