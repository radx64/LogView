#include "ProjectModel.hpp"

#include <memory>

#include "BookmarksModel.hpp"
#include "Logfile.hpp"
#include "GrepNode.hpp"

ProjectModel::ProjectModel()
{
    bookmarks_model_ = std::make_unique<BookmarksModel>(nullptr);
}

