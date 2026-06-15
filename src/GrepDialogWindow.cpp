#include "GrepDialogWindow.hpp"
#include "ui_GrepDialogWindow.h"

#include <QDebug>
#include <QRegularExpression>

GrepDialogWindow::GrepDialogWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrepDialogWindow)
{
    ui->setupUi(this);
}

GrepDialogWindow::~GrepDialogWindow()
{
    delete ui;
}

GrepDialogWindow::Result GrepDialogWindow::getResult()
{
    Result result;
    result.pattern = ui->pattern->text();
    result.is_regex = ui->regex_check->isChecked();
    result.is_case_insensitive = ui->case_insensitive_check->isChecked();
    result.is_inverted = ui->inverted_check->isChecked();
    return result;
}

void GrepDialogWindow::on_button_clicked()
{
    accept();
}

void GrepDialogWindow::on_regex_check_clicked()
{
    if (ui->regex_check->isChecked())
    {
        ui->case_insensitive_check->setEnabled(false);
        on_pattern_textEdited(ui->pattern->text());
    }
    if (!ui->regex_check->isChecked())
    {
        ui->case_insensitive_check->setEnabled(true);
        ui->pattern->setPalette(QPalette());
    }
}

void GrepDialogWindow::on_pattern_textEdited(const QString &arg1)
{
    if (ui->regex_check->isChecked())
    {
        QPalette pallete = ui->pattern->palette();
        QRegularExpression expression(arg1);
        const QColor background = expression.isValid()
                                      ? QColor(Qt::green).lighter()
                                      : QColor(Qt::red).lighter();
        pallete.setColor(QPalette::Base, background);
        pallete.setColor(QPalette::Text, Qt::black);
        ui->pattern->setPalette(pallete);
    }
}
