#include "BookmarkDialogWindow.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSize>

namespace
{
struct IconEntry
{
    const char* label;
    const char* path;
};

const IconEntry kIcons[] = {
    {"Bookmark", ":/icon/Gnome-Bookmark-New-32.png"},
    {"Warning", ":/icon/Gnome-Dialog-Warning-32.png"},
    {"Indent", ":/icon/Gnome-Format-Indent-More-32.png"},
    {"Log", ":/icon/Gnome-Logviewer-32.png"},
    {"Apply", ":/icon/Dialog-Apply.png"},
    {"Open", ":/icon/Gnome-Document-Open-32.png"},
    {"Save", ":/icon/Gnome-Document-Save-32.png"},
};

const char* kDefaultIcon = ":/icon/Gnome-Bookmark-New-32.png";
}

BookmarkDialogWindow::BookmarkDialogWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Bookmark"));
    resize(400, 120);

    QGridLayout* outer = new QGridLayout(this);
    QGridLayout* form = new QGridLayout();
    form->setSpacing(6);

    QLabel* nameLabel = new QLabel(tr("Name"), this);
    nameLabel->setMinimumSize(QSize(50, 0));
    name_ = new QLineEdit(this);
    QLabel* iconLabel = new QLabel(tr("Icon"), this);
    icon_combo_ = new QComboBox(this);

    form->addWidget(nameLabel, 0, 0, 1, 1);
    form->addWidget(name_, 0, 1, 1, 1);
    form->addWidget(iconLabel, 1, 0, 1, 1);
    form->addWidget(icon_combo_, 1, 1, 1, 1);

    button_ = new QPushButton(tr("Save"), this);

    outer->addLayout(form, 0, 0, 1, 1);
    outer->addWidget(button_, 1, 0, 1, 1);

    setTabOrder(name_, icon_combo_);
    setTabOrder(icon_combo_, button_);

    connect(button_, &QPushButton::clicked, this, &BookmarkDialogWindow::on_button_clicked);

    populateIcons();
}

BookmarkDialogWindow::~BookmarkDialogWindow() = default;

void BookmarkDialogWindow::populateIcons()
{
    for (const auto& entry : kIcons)
    {
        const QString path = QString::fromLatin1(entry.path);
        icon_combo_->addItem(QIcon(path), QString::fromLatin1(entry.label), path);
    }
}

void BookmarkDialogWindow::setName(const QString& name)
{
    name_->setText(name);
    name_->selectAll();
}

void BookmarkDialogWindow::setIcon(const QString& icon_path)
{
    const int index = icon_combo_->findData(icon_path);
    if (index >= 0) icon_combo_->setCurrentIndex(index);
}

BookmarkDialogWindow::Result BookmarkDialogWindow::getResult()
{
    Result result;
    result.name = name_->text();
    result.icon = icon_combo_->currentData().toString();
    if (result.icon.isEmpty()) result.icon = QString::fromLatin1(kDefaultIcon);
    return result;
}

void BookmarkDialogWindow::on_button_clicked()
{
    accept();
}
