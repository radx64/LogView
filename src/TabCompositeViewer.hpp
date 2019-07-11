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
    TabCompositeViewer(QWidget* parent);
    void grep(QString pattern);
    void setContent(const Lines& lines);

    Lines lines_;
    QTabWidget* tabs_;

public slots:
    void closeTab(const int);
};

#endif // TAB_COMPOSITE_VIEWER_HPP
