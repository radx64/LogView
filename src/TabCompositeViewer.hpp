#ifndef TAB_COMPOSITE_VIEWER_HPP
#define TAB_COMPOSITE_VIEWER_HPP

#include "Viewer.hpp"

#include <QStringList>

class QWidget;
class QTabWidget;

class TabCompositeViewer : public Viewer
{
    Q_OBJECT
public:
    TabCompositeViewer(QWidget* parent);
    void grep(QString pattern);

    QStringList lines_;
    QTabWidget* tabs_;

public slots:
    void closeTab(const int);
};

#endif // TAB_COMPOSITE_VIEWER_HPP
