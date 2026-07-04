#pragma once

#include <QDialog>
#include <QColor>
#include <QFont>

class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;
class QFontComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    QWidget* createGeneralPage();
    QWidget* createEditorPage();
    QWidget* createAutoMarkingsPage();
    QWidget* createAutoBookmarksPage();
    QWidget* createUpdatesPage();
    void addSection(const QString& title, QWidget* page);
    void loadValues();
    void applyChanges();
    void applyColorButtonStyle(QPushButton* button, const QColor& color);
    void updateColorButton();
    void pickColor(QColor& target, const QString& title, const QColor& fallback);
    void pickHighlightColor();

    QTreeWidget* section_tree_{nullptr};
    QStackedWidget* pages_{nullptr};

    QFontComboBox* font_combo_{nullptr};
    QSpinBox* font_size_spin_{nullptr};
    QPushButton* highlight_color_button_{nullptr};
    QPushButton* highlight_color_reset_{nullptr};
    QPushButton* search_color_button_{nullptr};
    QPushButton* search_color_reset_{nullptr};
    QPushButton* search_current_color_button_{nullptr};
    QPushButton* search_current_color_reset_{nullptr};
    QCheckBox* check_updates_checkbox_{nullptr};
    QCheckBox* prompt_save_on_exit_checkbox_{nullptr};

    QColor highlight_color_{};
    QColor search_color_{};
    QColor search_current_color_{};
};
