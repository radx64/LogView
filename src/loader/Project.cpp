#include "Project.hpp"

#include <memory>

#include "ui_MainWindow.h"

#include "../ProjectModel.hpp"

#include "LoaderLogFile.hpp"

namespace loader
{

void Project::load(Ui::MainWindow *ui, ::ProjectModel* pm)
{
    for (const auto& logfile : pm->logfiles_)
    {
        Logfile::load(ui, logfile.get());
    }
}
}  // namespace loader
