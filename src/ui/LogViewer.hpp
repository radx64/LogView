#pragma once

#include <functional>
#include <memory>

#include <QString>
#include <QWidget>

class QTabWidget;
class ChunkedTextView;
class GrepNode;
class LineSource;
class FilteredLineSource;
class MarkingsModel;

class LogViewer : public QWidget
{
Q_OBJECT
public:
    LogViewer(QWidget* parent, GrepNode* grep_node, std::shared_ptr<LineSource> source,
              MarkingsModel* markings = nullptr);
    LogViewer* grep(GrepNode* grep, std::function<void(LogViewer*)> on_ready = {});

    void refresh();

    GrepNode* getGrepNode();

    LineSource* source() const { return source_.get(); }

    QTabWidget* tabs_;
    ChunkedTextView* text_;

public slots:
    void closeTab(const int);

private slots:
    void showTabContextMenu(const QPoint& pos);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void startFilter(LogViewer* child, std::shared_ptr<FilteredLineSource> source,
                     const QString& base_name, std::function<void(LogViewer*)> on_ready);

    std::shared_ptr<LineSource> source_;
    GrepNode* grep_node_;
    MarkingsModel* markings_;
};
