#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QComboBox;

class MarkingDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MarkingDialogWindow(QWidget *parent = nullptr);

    struct Result
    {
        QString text{};
        QString color{};
    };

    void setText(const QString& text);
    void setColor(const QString& color);

    Result getResult() const;

private:
    void populateColors();

    QLineEdit* text_edit_;
    QComboBox* color_combo_;
};
