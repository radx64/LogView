#pragma once

#include "ArchiveDocument.hpp"

#include <QJsonObject>
#include <QPointer>
#include <QWidget>

class FileViewer;
class LogViewer;
class Logfile;
class ProjectUiManager;
class QTabWidget;
class QTreeView;
class QLabel;
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

private slots:
    void onCatalogUpdated();
    void onInnerTabCloseRequested(int index);
    void onTreeActivated(const QModelIndex& index);
    void showTreeContextMenu(const QPoint& pos);

private:
    void setupArchiveTab();
    void addInnerFileViewer(Logfile* lf, const QString& tab_label);
    int innerTabIndexForEntry(const QString& entry_id) const;

    QPointer<ArchiveDocument> archive_;
    ProjectUiManager* project_ui_;
    QTabWidget* tabs_;
    QWidget* archive_tab_;
    QTreeView* tree_;
    ZipEntryTreeModel* tree_model_;
    QLabel* catalog_status_;
};
