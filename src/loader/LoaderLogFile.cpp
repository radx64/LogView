#include "LoaderLogFile.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"

#include "../TabCompositeViewer.hpp"
#include "../Viewer.hpp"
#include "ui_MainWindow.h"

#include <QtDebug>

namespace loader
{

void Logfile::load(Ui::MainWindow *ui, ::Logfile* lf)
{
    QTabWidget* file_tab_widget = ui->fileView;
    Viewer* viewer = new Viewer(file_tab_widget, lf);
    file_tab_widget ->addTab(viewer, lf->getFileName().split(QRegularExpression("[\\/]")).last());
    spawnViews(viewer->getDeepestActiveTab(), lf->grep_hierarchy_.get());
}

void Logfile::spawnViews(TabCompositeViewer* parent_tab, const GrepNode* node)
{
    if (node == nullptr) return;
    for (GrepNode* child : node->getChildren())
    {
        TabCompositeViewer* spawned_viewer = parent_tab->grep(child);
        spawnViews(spawned_viewer, child);
    }
}
}  // namespace loader
