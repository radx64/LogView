#ifndef VIEWER_WIDGET_HPP
#define VIEWER_WIDGET_HPP

#include <memory>
#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class Logfile;
class ProjectModel;
class QListView;
class TabCompositeViewer;

class Viewer: public QWidget
{
public:
    Viewer(QWidget* parent, std::unique_ptr<Logfile> log);
    Viewer(QWidget* parent, std::unique_ptr<ProjectModel> project_model);
    TabCompositeViewer* getDeepestActiveTab();

    QListView* bookmarks_widget_;
    std::unique_ptr<ProjectModel> project_model_;

protected:
    QHBoxLayout* layout_;
    TabCompositeViewer* logViewer_;

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // VIEWER_WIDGET_HPP
