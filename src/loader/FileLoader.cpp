#include "FileLoader.hpp"

#include <QElapsedTimer>
#include <QThread>

#include "../FileLineSource.hpp"

FileLoader::FileLoader(std::shared_ptr<FileLineSource> source, QObject* parent)
    : QObject(parent), source_(std::move(source))
{
}

FileLoader::~FileLoader()
{
    cancel();
    if (thread_)
    {
        thread_->wait();
        delete thread_;
    }
}

void FileLoader::start()
{
    if (thread_) return;

    thread_ = QThread::create([this]()
    {
        constexpr qint64 kMinIntervalMs = 250;
        QElapsedTimer timer;
        timer.start();
        qint64 last_emit_ms = -1;

        const bool ok = source_->buildIndex(
            [this, &timer, &last_emit_ms](qint64 done, qint64 total)
        {
            const qint64 now = timer.elapsed();
            const bool finished = (total <= 0) || (done >= total);
            if (finished || last_emit_ms < 0 || (now - last_emit_ms) >= kMinIntervalMs)
            {
                last_emit_ms = now;
                emit progress(done, total);
            }
            return !cancel_.load(std::memory_order_relaxed);
        });
        emit finished(ok);
    });
    thread_->start();
}

void FileLoader::cancel()
{
    cancel_.store(true, std::memory_order_relaxed);
}
