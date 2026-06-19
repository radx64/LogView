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
    // Creates the FileViewer/tab for a logfile and returns it. Does not spawn
    // the grep hierarchy views; that is deferred until the backing source has
    // finished loading (see spawnViews).
    static FileViewer* createView(Ui::MainWindow *ui, ::Logfile* pm, std::function<void(FileViewer*)> connect_slots_method);
    static void spawnViews(LogViewer* parent_tab, const GrepNode* node);
};

}  // namespace loader
