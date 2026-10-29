#pragma once

#include <functional>
#include <memory>

class FileViewer;
class LogViewer;
class Logfile;
class GrepNode;
class ArchiveDocument;
class ArchiveViewer;
class ProjectUiManager;
namespace Ui { class MainWindow; }

namespace loader
{

class Logfile
{
public:
    Logfile() = delete;
    static FileViewer* createView(Ui::MainWindow *ui, ::Logfile* pm, std::function<void(FileViewer*)> connect_slots_method);
    static ArchiveViewer* createArchiveView(Ui::MainWindow* ui,
                                            ArchiveDocument* archive,
                                            ProjectUiManager* project_ui,
                                            std::function<void(ArchiveViewer*)> connect_slots_method);
    static void spawnViews(LogViewer* parent_tab, const GrepNode* node);
};

}  // namespace loader
