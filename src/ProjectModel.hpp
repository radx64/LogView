#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <QString>

class BookmarksModel;
class GrepNode;
class Logfile;

namespace serializer { class ProjectModel; }

class ProjectModel
{
public:
    ProjectModel();

    BookmarksModel* getBookmarksModel();

    QString file_path_;
    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<Logfile> logfile_model_;
protected:
    std::unique_ptr<BookmarksModel> bookmarks_model_;

    friend class serializer::ProjectModel;

};

#endif // PROJECT_MODEL_HPP
