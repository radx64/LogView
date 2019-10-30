#ifndef VIEWER_WIDGET_HPP
#define VIEWER_WIDGET_HPP

#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class Logfile;
class QListView;
class TabCompositeViewer;

class Viewer: public QWidget
{
public:
    Viewer(QWidget* parent, Logfile* logfile);
    TabCompositeViewer* getDeepestActiveTab();

    QListView* bookmarks_widget_;
    Logfile* logfile_; //TODO make this protected

protected:
    QHBoxLayout* layout_;
    TabCompositeViewer* logViewer_;

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // VIEWER_WIDGET_HPP
