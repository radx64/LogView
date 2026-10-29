#include "LoaderLogFile.hpp"

#include <memory>

#include <QFileInfo>
#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"
#include "../Logfile.hpp"
#include "../ArchiveDocument.hpp"
#include "../ProjectUiManager.hpp"

#include "LogViewer.hpp"
#include "FileViewer.hpp"
#include "ArchiveViewer.hpp"
#include "UiMainWindow.hpp"

#include <QtDebug>

namespace loader
{

FileViewer* Logfile::createView(Ui::MainWindow *ui, ::Logfile* lf, std::function<void(FileViewer*)> connect_slots_to_manager)
{
    QTabWidget* file_viewer_widget = ui->fileView;
    FileViewer* viewer = new FileViewer(file_viewer_widget, lf);
    connect_slots_to_manager(viewer);

    int tab_index = file_viewer_widget ->addTab(viewer, lf->getFileName().split(QRegularExpression("[\\/]")).last());
    file_viewer_widget ->setTabToolTip(tab_index, lf->getFileName());
    file_viewer_widget ->setCurrentIndex(tab_index);
    return viewer;
}

ArchiveViewer* Logfile::createArchiveView(Ui::MainWindow* ui,
                                          ArchiveDocument* archive,
                                          ProjectUiManager* project_ui,
                                          std::function<void(ArchiveViewer*)> connect_slots_method)
{
    QTabWidget* file_viewer_widget = ui->fileView;
    ArchiveViewer* viewer = new ArchiveViewer(file_viewer_widget, archive, project_ui);
    connect_slots_method(viewer);

    const QString tab_name = QFileInfo(archive->archivePath()).fileName();
    const int tab_index = file_viewer_widget->addTab(viewer, tab_name);
    file_viewer_widget->setTabToolTip(tab_index, archive->archivePath());
    file_viewer_widget->setCurrentIndex(tab_index);
    return viewer;
}

void Logfile::spawnViews(LogViewer* parent_tab, const GrepNode* node)
{
    if (node == nullptr) return;
    for (GrepNode* child : node->getChildren())
    {
        parent_tab->grep(child, [child](LogViewer* spawned_viewer)
        {
            spawnViews(spawned_viewer, child);
        });
    }
}

}  // namespace loader
