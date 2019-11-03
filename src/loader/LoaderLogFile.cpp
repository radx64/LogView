#include "LoaderLogFile.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"

#include "../LogViewer.hpp"
#include "../ProjectViewer.hpp"
#include "ui_MainWindow.h"

#include <QtDebug>

namespace loader
{

void Logfile::load(Ui::MainWindow *ui, ::Logfile* lf)
{
    QTabWidget* file_tab_widget = ui->fileView;
    ProjectViewer* viewer = new ProjectViewer(file_tab_widget, lf);
    int tab_index = file_tab_widget ->addTab(viewer, lf->getFileName().split(QRegularExpression("[\\/]")).last());
    file_tab_widget ->setTabToolTip(tab_index, lf->getFileName());

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
