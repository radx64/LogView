#include "BookmarkDialogWindow.hpp"
#include "ui_BookmarkDialogWindow.h"

#include <QIcon>

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
    QDialog(parent),
    ui(new Ui::BookmarkDialogWindow)
{
    ui->setupUi(this);
    populateIcons();
}

BookmarkDialogWindow::~BookmarkDialogWindow()
{
    delete ui;
}

void BookmarkDialogWindow::populateIcons()
{
    for (const auto& entry : kIcons)
    {
        const QString path = QString::fromLatin1(entry.path);
        ui->icon_combo->addItem(QIcon(path), QString::fromLatin1(entry.label), path);
    }
}

void BookmarkDialogWindow::setName(const QString& name)
{
    ui->name->setText(name);
    ui->name->selectAll();
}

void BookmarkDialogWindow::setIcon(const QString& icon_path)
{
    const int index = ui->icon_combo->findData(icon_path);
    if (index >= 0) ui->icon_combo->setCurrentIndex(index);
}

BookmarkDialogWindow::Result BookmarkDialogWindow::getResult()
{
    Result result;
    result.name = ui->name->text();
    result.icon = ui->icon_combo->currentData().toString();
    if (result.icon.isEmpty()) result.icon = QString::fromLatin1(kDefaultIcon);
    return result;
}

void BookmarkDialogWindow::on_button_clicked()
{
    accept();
}
