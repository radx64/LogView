#include "LoaderLogFile.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"
#include "../Logfile.hpp"

#include "LogViewer.hpp"
#include "FileViewer.hpp"
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
