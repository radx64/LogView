#pragma once

#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings& instance();

    QFont editorFont() const { return editor_font_; }
    void setEditorFont(const QFont& font);

    QColor highlightLineColor() const { return highlight_line_color_; }
    void setHighlightLineColor(const QColor& color);

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
};
