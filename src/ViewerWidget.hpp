#ifndef VIEWER_WIDGET_HPP
#define VIEWER_WIDGET_HPP

#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class QListView;
class TabCompositeViewer;


class ViewerWidget: public QWidget
{
public:
    ViewerWidget(QWidget* parent);
    TabCompositeViewer* logViewer_;
    QListView* bookmarks_widget_;
    BookmarksModel* bookmarks_model_;

protected:
    QHBoxLayout* layout_;

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // VIEWER_WIDGET_HPP
