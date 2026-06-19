#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

class QListWidget;
class QPushButton;
class QToolButton;
class QCheckBox;
class QLineEdit;
class QDragEnterEvent;
class QDropEvent;

class MergerDialogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MergerDialogWindow(QWidget* parent = nullptr);
    ~MergerDialogWindow() override;

    void addFiles(const QStringList& paths);

    QString outputPath() const;
    bool shouldOpenResult() const;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onAddClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onBrowseClicked();
    void onSaveClicked();
    void onOutputEdited();
    void updateDefaultOutputPath();

private:
    void appendFile(const QString& path);
    QStringList collectFiles() const;
    bool mergeFiles(const QStringList& inputs, const QString& output);
    static QString defaultOutputFor(const QString& firstFile);

    QListWidget* file_list_{nullptr};
    QToolButton* move_up_{nullptr};
    QToolButton* move_down_{nullptr};
    QPushButton* add_button_{nullptr};
    QPushButton* delete_button_{nullptr};
    QPushButton* clear_button_{nullptr};
    QCheckBox* recycle_check_{nullptr};
    QLineEdit* output_path_{nullptr};
    QPushButton* browse_button_{nullptr};
    QCheckBox* open_result_check_{nullptr};
    QPushButton* save_button_{nullptr};
    QPushButton* cancel_button_{nullptr};

    bool output_user_edited_{false};
};
