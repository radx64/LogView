#include "Project.hpp"

#include <memory>

#include "ui_MainWindow.h"

#include "../ProjectModel.hpp"

#include "LoaderLogFile.hpp"

namespace loader
{

void Project::load(Ui::MainWindow *ui, ::ProjectModel* pm, std::function<void(::Logfile*)> on_wiget_destroy_action)
{
    for (const auto& logfile : pm->get_log_files())
    {
        ::Logfile* raw_logfile = logfile.get();
        Logfile::load(ui, raw_logfile, [=](){on_wiget_destroy_action(raw_logfile);});
    }
}
}  // namespace loader
