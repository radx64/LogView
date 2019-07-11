#ifndef VIEWER_WIDGET_HPP
#define VIEWER_WIDGET_HPP

#include <QWidget>

class QHBoxLayout;
class QListView;
class TabCompositeViewer;

class ViewerWidget: public QWidget
{
public:
    ViewerWidget(QWidget* parent);
    TabCompositeViewer* logViewer_;

protected:
    QWidget* parent_;
    QHBoxLayout* layout_;
    QListView* bookmarks_;
};

#endif // VIEWER_WIDGET_HPP
