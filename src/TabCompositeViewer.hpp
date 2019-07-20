#ifndef TAB_COMPOSITE_VIEWER_HPP
#define TAB_COMPOSITE_VIEWER_HPP

#include <QStringList>
#include <QWidget>

#include "Logfile.hpp"

class QWidget;
class QTabWidget;
class TextRenderer;

class TabCompositeViewer : public QWidget
{
    Q_OBJECT
public:
    TabCompositeViewer(QWidget* parent, const Lines lines);
    void grep(QString pattern);

    QTabWidget* tabs_;
    TextRenderer* text_;
// in further improvements I need to hold only grepped lines from log model not copy of lines itself;
    const Lines lines_;

public slots:
    void closeTab(const int);
};

#endif // TAB_COMPOSITE_VIEWER_HPP
