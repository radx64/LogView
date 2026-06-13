#pragma once

#include <QDialog>
#include <QColor>
#include <QString>

class QLineEdit;
class QPushButton;

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
    void pickColor();
    void updateColorButton();

    QLineEdit* text_edit_;
    QPushButton* color_button_;
    QColor color_{};
};
