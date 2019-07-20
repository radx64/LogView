#ifndef TAB_COMPOSITE_VIEWER_HPP
#define TAB_COMPOSITE_VIEWER_HPP

#include "Viewer.hpp"

#include <QStringList>

#include "Logfile.hpp"

class QWidget;
class QTabWidget;

class TabCompositeViewer : public Viewer
{
    Q_OBJECT
public:
    TabCompositeViewer(QWidget* parent, const Lines lines);
    void grep(QString pattern);

//    Lines lines_; // in further improvements I need to hold only grepped lines from log model;
    QTabWidget* tabs_;

public slots:
    void closeTab(const int);
};

#endif // TAB_COMPOSITE_VIEWER_HPP
