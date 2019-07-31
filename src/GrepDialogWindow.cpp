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
        QPalette pallete = ui->pattern->palette();
        pallete.setColor(QPalette::Base, Qt::white);
        ui->pattern->setPalette(pallete);
    }
}

void GrepDialogWindow::on_pattern_textEdited(const QString &arg1)
{
    if (ui->regex_check->isChecked())
    {
        QPalette pallete = ui->pattern->palette();
        QRegularExpression expression(arg1);
        if (expression.isValid()) pallete.setColor(QPalette::Base, QColor(Qt::green).lighter());
        else pallete.setColor(QPalette::Base, QColor(Qt::red).lighter());
        ui->pattern->setPalette(pallete);
    }
}
