#ifndef GREPDIALOGWINDOW_HPP
#define GREPDIALOGWINDOW_HPP

#include <QDialog>

namespace Ui {
class GrepDialogWindow;
}

class GrepDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GrepDialogWindow(QWidget *parent = nullptr);
    ~GrepDialogWindow();

    struct Result
    {
        QString pattern;
        bool is_regex;
        bool is_case_insensitive;
        bool is_inverted;
    };

    Result getResult();

private slots:
    void on_button_clicked();

    void on_regex_check_clicked();

    void on_pattern_textEdited(const QString &arg1);

private:
    Ui::GrepDialogWindow *ui;
};

#endif // GREPDIALOGWINDOW_HPP
