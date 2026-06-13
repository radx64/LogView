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

// Application configuration window.
// Uses a section tree on the left and a stacked page area on the right so
// new configuration sections can be added with minimal changes.
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    QWidget* createEditorPage();
    void addSection(const QString& title, QWidget* page);
    void loadValues();
    void applyChanges();
    void updateColorButton();
    void pickHighlightColor();

    QTreeWidget* section_tree_{nullptr};
    QStackedWidget* pages_{nullptr};

    // Editor page widgets.
    QFontComboBox* font_combo_{nullptr};
    QSpinBox* font_size_spin_{nullptr};
    QPushButton* highlight_color_button_{nullptr};
    QPushButton* highlight_color_reset_{nullptr};

    QColor highlight_color_{};
};
