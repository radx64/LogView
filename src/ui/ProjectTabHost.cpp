#include "ProjectTabHost.hpp"

#include "ArchiveViewer.hpp"
#include "FileViewer.hpp"
#include "Logfile.hpp"

FileViewer* activeFileViewerForTab(QWidget* tab_widget)
{
    if (tab_widget == nullptr)
        return nullptr;
    if (auto* viewer = qobject_cast<FileViewer*>(tab_widget))
        return viewer;
    if (auto* archive = qobject_cast<ArchiveViewer*>(tab_widget))
        return archive->activeFileViewer();
    return nullptr;
}

QString primaryPathForTab(QWidget* tab_widget)
{
    if (tab_widget == nullptr)
        return QString();
    if (auto* viewer = qobject_cast<FileViewer*>(tab_widget))
    {
        if (viewer->logfile_ != nullptr)
            return viewer->logfile_->getStoragePath();
    }
    if (auto* archive = qobject_cast<ArchiveViewer*>(tab_widget))
        return archive->archivePath();
    return QString();
}
