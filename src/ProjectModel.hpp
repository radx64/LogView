#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <QString>

class BookmarksModel;
class GrepNode;
class Logfile;

class ProjectModel
{
public:
    ProjectModel();

    QString filePath_;
    std::unique_ptr<GrepNode> grepHierarchy_;
    std::unique_ptr<Logfile> logfile_model_;
    std::unique_ptr<BookmarksModel> bookmarks_model_;
};

#endif // PROJECT_MODEL_HPP
