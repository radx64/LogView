#pragma once

#include "ArchiveDocument.hpp"

#include <QJsonObject>
#include <QPointer>
#include <QStringList>
#include <QWidget>

class FileViewer;
class LogViewer;
class Logfile;
class ProjectUiManager;
class QTabWidget;
class QTreeView;
class QLabel;
class QLineEdit;
class QProgressBar;
class QTimer;
class ZipEntryTreeModel;

class ArchiveViewer : public QWidget
{
Q_OBJECT
public:
    ArchiveViewer(QWidget* parent, ArchiveDocument* archive, ProjectUiManager* project_ui);
    ~ArchiveViewer() override;

    ArchiveDocument* archiveDocument() const { return archive_.data(); }
    QString archivePath() const;

    FileViewer* activeFileViewer() const;
    LogViewer* getDeepestActiveTab();

    void openEntry(const QString& entry_id);
    void restorePendingEntries();
    void setCatalogProgress(int entries, qint64 bytes_read, qint64 total_bytes);

private slots:
    void onCatalogUpdated();
    void onInnerTabCloseRequested(int index);
    void onTreeActivated(const QModelIndex& index);
    void showTreeContextMenu(const QPoint& pos);
    void showFileTabContextMenu(const QPoint& pos);
    void scheduleTreeFilter();
    void applyTreeFilter();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupArchiveTab();
    void addInnerFileViewer(Logfile* lf, const QString& tab_label);
    int innerTabIndexForEntry(const QString& entry_id) const;
    void updateCatalogStatusLabel(int visible_count);

    QPointer<ArchiveDocument> archive_;
    ProjectUiManager* project_ui_;
    QTabWidget* tabs_;
    QWidget* archive_tab_;
    QTreeView* tree_;
    ZipEntryTreeModel* tree_model_;
    QLineEdit* filter_edit_;
    QProgressBar* catalog_progress_;
    QTimer* filter_debounce_;
    QLabel* catalog_status_;
    QStringList catalog_paths_;
    bool catalog_truncated_ = false;
};
