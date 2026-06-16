#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QPushButton;
class QCheckBox;

class GrepDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GrepDialogWindow(QWidget *parent = nullptr);
    ~GrepDialogWindow() override;

    struct Result
    {
        QString pattern{};
        bool is_regex{};
        bool is_case_sensitive{};
        bool is_inverted{};
    };

    Result getResult();

private slots:
    void on_button_clicked();
    void on_regex_check_clicked();
    void on_pattern_textEdited(const QString &arg1);

private:
    QLineEdit* pattern_{nullptr};
    QPushButton* button_{nullptr};
    QCheckBox* regex_check_{nullptr};
    QCheckBox* case_sensitive_check_{nullptr};
    QCheckBox* inverted_check_{nullptr};

    static bool last_regex_;
    static bool last_case_sensitive_;
    static bool last_inverted_;
};
