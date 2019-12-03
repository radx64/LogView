#ifndef LOADER_LOGFILE_HPP
#define LOADER_LOGFILE_HPP

#include <functional>
#include <memory>

class LogViewer;
class Logfile;
class GrepNode;
namespace Ui { class MainWindow; }

namespace loader
{

class Logfile
{
public:
    Logfile() = delete;
    static void load(Ui::MainWindow *ui, ::Logfile* pm, std::function<void()> on_ui_destroy_action);

protected:
    static void spawnViews(LogViewer* parent_tab, const GrepNode* node);
};

}  // namespace loader

#endif // LOADER_LOGFILE_HPP
