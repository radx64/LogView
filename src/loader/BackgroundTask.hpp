#pragma once

#include <atomic>
#include <functional>

#include <QObject>

class QThread;

// Runs an arbitrary long-running job on a background thread and reports
// progress/completion back on the thread that owns the task (the GUI thread)
// through queued signals. Used for the file-index scan and for grep filtering
// so the UI stays responsive while large sources are processed.
class BackgroundTask : public QObject
{
Q_OBJECT
public:
    using ProgressFn = std::function<bool(qint64 done, qint64 total)>;
    using Job = std::function<bool(const ProgressFn&)>;

    explicit BackgroundTask(Job job, QObject* parent = nullptr);
    ~BackgroundTask() override;

    void start();
    void cancel();

signals:
    void progress(qint64 done, qint64 total);
    void finished(bool ok);

private:
    Job job_;
    QThread* thread_ = nullptr;
    std::atomic<bool> cancel_{false};
};
