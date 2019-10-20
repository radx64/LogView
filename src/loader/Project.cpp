#include "Project.hpp"

#include <memory>

#include <QRegularExpression>

#include "../BookmarksModel.hpp"
#include "../GrepNode.hpp"
#include "../ProjectModel.hpp"
#include "../TabCompositeViewer.hpp"
#include "../Viewer.hpp"
#include "../ui_MainWindow.h"

namespace loader
{

void Project::load(Ui::MainWindow *ui, std::unique_ptr<::ProjectModel> pm)
{
    QTabWidget* file_tab_widget = ui->fileView;
    QString filename = pm->file_path_;
    Viewer* viewer = new Viewer(file_tab_widget, std::move(pm));
    file_tab_widget ->addTab(viewer, filename.split(QRegularExpression("[\\/]")).last());
    spawnGreppedViews(viewer->getDeepestActiveTab(), viewer->project_model_->grep_hierarchy_.get());
}

void Project::spawnGreppedViews(TabCompositeViewer* parent_tab, const GrepNode* node)
{
    for (GrepNode* child : node->getChildren())
    {
        TabCompositeViewer* spawned_viewer = parent_tab->grep(
            QString().fromStdString(child->getValue()), false, false); //todo read rest parameters from GrepNode when implemented
        spawnGreppedViews(spawned_viewer, child);
    }
}

}  // namespace loader
