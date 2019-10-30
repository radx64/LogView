#ifndef LOADER_PROJECT_HPP
#define LOADER_PROJECT_HPP

#include <memory>

class TabCompositeViewer;
class ProjectModel;
class GrepNode;
namespace Ui { class MainWindow; }

namespace loader
{

class Project
{
public:
    Project() = delete;
    static void load(Ui::MainWindow *ui, ::ProjectModel* pm);
};

}  // namespace loader

#endif // LOADER_PROJECT_HPP
