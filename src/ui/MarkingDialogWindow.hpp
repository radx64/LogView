#pragma once

#include <QDialog>
#include <QColor>
#include <QString>

class QLineEdit;
class QPushButton;
class QCheckBox;

class MarkingDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MarkingDialogWindow(QWidget *parent = nullptr);

    struct Result
    {
        QString text{};
        QString color{};
        QString text_color{};
    };

    void setText(const QString& text);
    void setColor(const QString& color);
    void setTextColor(const QString& textColor);

    Result getResult() const;

private:
    void pickColor();
    void pickTextColor();
    void updateColorButton();
    void updateTextColorButton();

    QLineEdit* text_edit_;
    QPushButton* color_button_;
    QPushButton* text_color_button_;
    QCheckBox* auto_text_color_;
    QColor color_{};
    QColor text_color_{};
};
