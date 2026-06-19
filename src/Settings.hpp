#pragma once

#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>
#include <QStringList>

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings& instance();

    QFont editorFont() const { return editor_font_; }
    void setEditorFont(const QFont& font);

    QColor highlightLineColor() const { return highlight_line_color_; }
    void setHighlightLineColor(const QColor& color);

    bool checkForUpdatesOnStartup() const { return check_updates_on_startup_; }
    void setCheckForUpdatesOnStartup(bool enabled);

    QString skippedUpdateVersion() const { return skipped_update_version_; }
    void setSkippedUpdateVersion(const QString& version);

    static constexpr int kMaxRecent = 10;

    QString lastOpenedFileDirectory() const { return last_file_dir_; }
    void setLastOpenedFileDirectory(const QString& directory);

    QString lastOpenedProjectDirectory() const { return last_project_dir_; }
    void setLastOpenedProjectDirectory(const QString& directory);

    QStringList recentFiles() const { return recent_files_; }
    QStringList recentProjects() const { return recent_projects_; }

    void addRecentFile(const QString& path);
    void addRecentProject(const QString& path);
    void removeRecentFile(const QString& path);
    void removeRecentProject(const QString& path);

    void load();
    void save() const;

    static QString filePath();

signals:
    void changed();

private:
    explicit Settings(QObject* parent = nullptr);
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    static QFont defaultEditorFont();

    QFont editor_font_;
    QColor highlight_line_color_{};
    bool check_updates_on_startup_{true};
    QString skipped_update_version_{};
    QString last_file_dir_{};
    QString last_project_dir_{};
    QStringList recent_files_{};
    QStringList recent_projects_{};
};
