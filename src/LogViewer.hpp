#ifndef LOG_VIEWER_HPP
#define LOG_VIEWER_HPP

#include <memory>

#include <QWidget>

class QTabWidget;
class ChunkedTextView;
class GrepNode;
class LineSource;

class LogViewer : public QWidget
{
Q_OBJECT
public:
    LogViewer(QWidget* parent, GrepNode* grep_node, std::shared_ptr<LineSource> source);
    LogViewer* grep(GrepNode* grep);

    GrepNode* getGrepNode();

    LineSource* source() const { return source_.get(); }

    QTabWidget* tabs_;
    ChunkedTextView* text_;

public slots:
    void closeTab(const int);

protected:
    std::shared_ptr<LineSource> source_;
    GrepNode* grep_node_;
};

#endif // LOG_VIEWER_HPP
