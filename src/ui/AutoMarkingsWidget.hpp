#pragma once

#include <QWidget>

class QModelIndex;
class QPushButton;
class QTableView;

class AutoMarkingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AutoMarkingsWidget(QWidget* parent = nullptr);

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
