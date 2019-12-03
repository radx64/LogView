#ifndef LOADER_PROJECT_HPP
#define LOADER_PROJECT_HPP

#include <functional>
#include <memory>

class Logfile;
class LogViewer;
class ProjectModel;
class GrepNode;
namespace Ui { class MainWindow; }

namespace loader
{

class Project
{
public:
    Project() = delete;
    static void load(Ui::MainWindow *ui, ::ProjectModel* pm, std::function<void(::Logfile*)> on_wiget_destroy_action);
};

}  // namespace loader

#endif // LOADER_PROJECT_HPP
