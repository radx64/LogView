#pragma once

#include <functional>
#include <memory>

class FileViewer;
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
    static void load(Ui::MainWindow *ui, ::Logfile* pm, std::function<void(FileViewer*)> connect_slots_method);
    static void spawnViews(LogViewer* parent_tab, const GrepNode* node);
};

}  // namespace loader
