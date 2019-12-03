#include "Project.hpp"

#include <memory>

#include "ui_MainWindow.h"

#include "../ProjectModel.hpp"

#include "LoaderLogFile.hpp"

namespace loader
{

void Project::load(Ui::MainWindow *ui, ::ProjectModel* pm, std::function<void(::FileViewer*)> connect_slots_to_manager)
{
    for (const auto& logfile : pm->get_log_files())
    {
        ::Logfile* raw_logfile = logfile.get();
        Logfile::load(ui, raw_logfile, connect_slots_to_manager);
    }
}
}  // namespace loader
