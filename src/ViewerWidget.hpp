#ifndef VIEWER_WIDGET_HPP
#define VIEWER_WIDGET_HPP

#include <memory>
#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class QListView;
class TabCompositeViewer;
class Logfile;
class Viewer;

class ViewerWidget: public QWidget
{
public:
    ViewerWidget(QWidget* parent, std::unique_ptr<Logfile> log);
    TabCompositeViewer* getDeepestActiveTab();

    QListView* bookmarks_widget_;
    BookmarksModel* bookmarks_model_;

protected:
    QHBoxLayout* layout_;
    TabCompositeViewer* logViewer_;
    std::unique_ptr<Logfile> logfile_model_;

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // VIEWER_WIDGET_HPP
