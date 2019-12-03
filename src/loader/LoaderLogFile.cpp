#include "LoaderLogFile.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"

#include "../LogViewer.hpp"
#include "../FileViewer.hpp"
#include "ui_MainWindow.h"

#include <QtDebug>

namespace loader
{

void Logfile::load(Ui::MainWindow *ui, ::Logfile* lf, std::function<void()> on_ui_destroy_action)
{
    QTabWidget* file_viewer_widget = ui->fileView;
    FileViewer* viewer = new FileViewer(file_viewer_widget, lf, on_ui_destroy_action);
    int tab_index = file_viewer_widget ->addTab(viewer, lf->getFileName().split(QRegularExpression("[\\/]")).last());
    file_viewer_widget ->setTabToolTip(tab_index, lf->getFileName());
    spawnViews(viewer->getDeepestActiveTab(), lf->grep_hierarchy_.get());
}

void Logfile::spawnViews(LogViewer* parent_tab, const GrepNode* node)
{
    if (node == nullptr) return;
    for (GrepNode* child : node->getChildren())
    {
        LogViewer* spawned_viewer = parent_tab->grep(child);
        spawnViews(spawned_viewer, child);
    }
}

}  // namespace loader
