#include "GrepDialogWindow.hpp"

#include "Translator.hpp"

#include <QCheckBox>
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QSize>

bool GrepDialogWindow::last_regex_ = false;
bool GrepDialogWindow::last_case_sensitive_ = false;
bool GrepDialogWindow::last_inverted_ = false;

GrepDialogWindow::GrepDialogWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(Lang::tr("grep.title"));
    resize(400, 121);

    QGridLayout* outer = new QGridLayout(this);
    QGridLayout* inner = new QGridLayout();

    QGridLayout* patternLayout = new QGridLayout();
    patternLayout->setSpacing(1);
    QLabel* label = new QLabel(Lang::tr("grep.pattern"), this);
    label->setMinimumSize(QSize(50, 0));
    pattern_ = new QLineEdit(this);
    patternLayout->addWidget(label, 0, 0, 1, 1);
    patternLayout->addWidget(pattern_, 0, 1, 1, 1);

    regex_check_ = new QCheckBox(Lang::tr("common.regex"), this);
    case_sensitive_check_ = new QCheckBox(Lang::tr("common.case_sensitive"), this);
    inverted_check_ = new QCheckBox(Lang::tr("grep.inverted"), this);
    inverted_check_->setFocusPolicy(Qt::StrongFocus);
    button_ = new QPushButton(Lang::tr("grep.title"), this);

    inner->addLayout(patternLayout, 0, 0, 1, 1);
    inner->addWidget(regex_check_, 1, 0, 1, 1);
    inner->addWidget(case_sensitive_check_, 2, 0, 1, 1);
    inner->addWidget(inverted_check_, 3, 0, 1, 1);
    inner->addWidget(button_, 5, 0, 1, 1);

    outer->addLayout(inner, 1, 0, 1, 1);

    setTabOrder(pattern_, regex_check_);
    setTabOrder(regex_check_, case_sensitive_check_);
    setTabOrder(case_sensitive_check_, button_);

    regex_check_->setChecked(last_regex_);
    case_sensitive_check_->setChecked(last_case_sensitive_);
    inverted_check_->setChecked(last_inverted_);

    connect(button_, &QPushButton::clicked, this, &GrepDialogWindow::on_button_clicked);
    connect(regex_check_, &QCheckBox::clicked, this, &GrepDialogWindow::on_regex_check_clicked);
    connect(pattern_, &QLineEdit::textEdited, this, &GrepDialogWindow::on_pattern_textEdited);

    on_regex_check_clicked();
}

GrepDialogWindow::~GrepDialogWindow() = default;

GrepDialogWindow::Result GrepDialogWindow::getResult()
{
    Result result;
    result.pattern = pattern_->text();
    result.is_regex = regex_check_->isChecked();
    result.is_case_sensitive = case_sensitive_check_->isChecked();
    result.is_inverted = inverted_check_->isChecked();
    return result;
}

void GrepDialogWindow::on_button_clicked()
{
    last_regex_ = regex_check_->isChecked();
    last_case_sensitive_ = case_sensitive_check_->isChecked();
    last_inverted_ = inverted_check_->isChecked();
    accept();
}

void GrepDialogWindow::on_regex_check_clicked()
{
    if (regex_check_->isChecked())
    {
        case_sensitive_check_->setEnabled(false);
        on_pattern_textEdited(pattern_->text());
    }
    if (!regex_check_->isChecked())
    {
        case_sensitive_check_->setEnabled(true);
        pattern_->setPalette(QPalette());
    }
}

void GrepDialogWindow::on_pattern_textEdited(const QString &arg1)
{
    if (regex_check_->isChecked())
    {
        QPalette pallete = pattern_->palette();
        QRegularExpression expression(arg1);
        const QColor background = expression.isValid()
                                      ? QColor(Qt::green).lighter()
                                      : QColor(Qt::red).lighter();
        pallete.setColor(QPalette::Base, background);
        pallete.setColor(QPalette::Text, Qt::black);
        pattern_->setPalette(pallete);
    }
}
