#include "Project.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"
#include "../ProjectModel.hpp"
#include "../TabCompositeViewer.hpp"
#include "../Viewer.hpp"
#include "ui_MainWindow.h"

namespace loader
{

void Project::load(Ui::MainWindow *ui, ::ProjectModel* pm)
{
    QTabWidget* file_tab_widget = ui->fileView;
    QString filename = pm->file_path_;
    Viewer* viewer = new Viewer(file_tab_widget, pm);
    file_tab_widget ->addTab(viewer, filename.split(QRegularExpression("[\\/]")).last());
    spawnGreppedViews(viewer->getDeepestActiveTab(), pm->grep_hierarchy_.get());
}

void Project::spawnGreppedViews(TabCompositeViewer* parent_tab, const GrepNode* node)
{
    if (node == nullptr) return;
    for (GrepNode* child : node->getChildren())
    {
        TabCompositeViewer* spawned_viewer = parent_tab->grep(child);
        spawnGreppedViews(spawned_viewer, child);
    }
}

}  // namespace loader
