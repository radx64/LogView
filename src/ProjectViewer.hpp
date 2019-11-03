#ifndef PROJECT_VIEWER_HPP
#define PROJECT_VIEWER_HPP

#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class Logfile;
class QListView;
class LogViewer;

class ProjectViewer: public QWidget
{
public:
    ProjectViewer(QWidget* parent, Logfile* logfile);
    LogViewer* getDeepestActiveTab();

    QListView* bookmarks_widget_;
    Logfile* logfile_; //TODO make this protected

protected:
    QHBoxLayout* layout_;
    LogViewer* logViewer_;

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // PROJECT_VIEWER_HPP
