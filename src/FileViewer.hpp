#ifndef PROJECT_VIEWER_HPP
#define PROJECT_VIEWER_HPP

#include <functional>

#include <QWidget>

class BookmarksModel;
class QHBoxLayout;
class Logfile;
class QListView;
class LogViewer;

class FileViewer: public QWidget
{
Q_OBJECT
public:
    FileViewer(QWidget* parent, Logfile* logfile);
    ~FileViewer();
    LogViewer* getDeepestActiveTab();

    QListView* bookmarks_widget_;
    Logfile* logfile_; //TODO make this protected

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    QHBoxLayout* layout_;
    LogViewer* logViewer_;

signals:
    void destroyed(Logfile* logfile);

private slots:
    void bookmarksItemDoubleClicked(const QModelIndex& idx);
};

#endif // PROJECT_VIEWER_HPP
