#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class BookmarkDialogWindow;
}

class BookmarkDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkDialogWindow(QWidget *parent = nullptr);
    ~BookmarkDialogWindow() override;

    struct Result
    {
        QString name{};
        QString icon{};
    };

    void setName(const QString& name);
    void setIcon(const QString& icon_path);

    Result getResult();

private slots:
    void on_button_clicked();

private:
    void populateIcons();

    Ui::BookmarkDialogWindow *ui;
};
