#include "BackgroundTask.hpp"

#include <QElapsedTimer>
#include <QThread>

BackgroundTask::BackgroundTask(Job job, QObject* parent)
    : QObject(parent), job_(std::move(job))
{
}

BackgroundTask::~BackgroundTask()
{
    cancel();
    if (thread_)
    {
        thread_->wait();
        delete thread_;
    }
}

void BackgroundTask::start()
{
    if (thread_) return;

    thread_ = QThread::create([this]()
    {
        constexpr qint64 kMinIntervalMs = 250;
        QElapsedTimer timer;
        timer.start();
        qint64 last_emit_ms = -1;

        const bool ok = job_([this, &timer, &last_emit_ms](qint64 done, qint64 total)
        {
            const qint64 now = timer.elapsed();
            const bool finished = (total <= 0) || (done >= total);
            if (finished || last_emit_ms < 0 || (now - last_emit_ms) >= kMinIntervalMs)
            {
                last_emit_ms = now;
                // Emitted from the worker thread; delivered to GUI-thread slots
                // via a queued connection.
                emit progress(done, total);
            }
            return !cancel_.load(std::memory_order_relaxed);
        });
        emit finished(ok);
    });
    thread_->start();
}

void BackgroundTask::cancel()
{
    cancel_.store(true, std::memory_order_relaxed);
}
