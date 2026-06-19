#pragma once

#include <functional>

#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class Logfile;
class QListView;
class LogViewer;

class FileViewer: public QWidget
{
Q_OBJECT
public:
    FileViewer(QWidget* parent, Logfile* logfile);
    ~FileViewer();
    LogViewer* getDeepestActiveTab();

    // Refreshes the base log view after its backing source finished loading.
    void refreshAfterLoad();

    QListView* bookmarks_widget_;
    QListView* markings_widget_;
    Logfile* logfile_; //TODO make this protected

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    QHBoxLayout* layout_;
    LogViewer* logViewer_;

signals:
    void destroyed(Logfile* logfile);

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
    void showBookmarksContextMenu(const QPoint& pos);
    void markingsItemDoubleClicked(const QModelIndex& idx);
    void showMarkingsContextMenu(const QPoint& pos);
};
