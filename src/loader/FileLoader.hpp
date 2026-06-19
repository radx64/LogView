#pragma once

#include <atomic>
#include <memory>

#include <QObject>

class FileLineSource;
class QThread;

// Drives FileLineSource::buildIndex() on a background thread and reports
// progress/completion back on the thread that owns the FileLoader (the GUI
// thread) through queued signals. This keeps the UI responsive while large
// files are scanned.
class FileLoader : public QObject
{
Q_OBJECT
public:
    explicit FileLoader(std::shared_ptr<FileLineSource> source, QObject* parent = nullptr);
    ~FileLoader() override;

    void start();
    void cancel();

signals:
    void progress(qint64 done, qint64 total);
    void finished(bool ok);

private:
    std::shared_ptr<FileLineSource> source_;
    QThread* thread_ = nullptr;
    std::atomic<bool> cancel_{false};
};
