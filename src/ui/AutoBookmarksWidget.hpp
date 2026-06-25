#pragma once

#include <QWidget>

class QListWidget;
class QModelIndex;
class QPushButton;
class QStandardItemModel;
class QTableView;

class AutoBookmarksWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AutoBookmarksWidget(QWidget* parent = nullptr);

private:
    QString currentFile() const;
    int selectedRuleRow() const;

    void refreshFileList();
    void refreshRuleTable();

    void addFile();
    void removeSelectedFile();

    void addRule();
    void editSelectedRule();
    void removeSelectedRule();

    void updateButtonState();

    QListWidget* file_list_{nullptr};
    QPushButton* remove_file_button_{nullptr};
    QTableView* rule_table_{nullptr};
    QStandardItemModel* rule_model_{nullptr};
    QPushButton* add_rule_button_{nullptr};
    QPushButton* edit_rule_button_{nullptr};
    QPushButton* remove_rule_button_{nullptr};
};
