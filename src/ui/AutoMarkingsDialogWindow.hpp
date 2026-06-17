#pragma once

#include <QDialog>

class QModelIndex;
class QPushButton;
class QTableView;

class AutoMarkingsDialogWindow : public QDialog
{
    Q_OBJECT
public:
    explicit AutoMarkingsDialogWindow(QWidget* parent = nullptr);

private:
    int selectedRow() const;
    void addMarking();
    void editSelectedMarking();
    void removeSelectedMarking();
    void updateButtonState();

    QTableView* table_{nullptr};
    QPushButton* edit_button_{nullptr};
    QPushButton* remove_button_{nullptr};
};
