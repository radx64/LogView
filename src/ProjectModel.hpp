#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <QString>

#include "Serializable.hpp"

class BookmarksModel;
class GrepNode;
class Logfile;

class ProjectModel : public Serializable
{
public:
    ProjectModel();

    BookmarksModel* getBookmarksModel();

    QString file_path_;
    std::unique_ptr<GrepNode> grep_hierarchy_;
    std::unique_ptr<Logfile> logfile_model_;

    virtual void serialize(QJsonObject &json) const override;
    virtual void deserialize(const QJsonObject &json) override;

protected:
    std::unique_ptr<BookmarksModel> bookmarks_model_;
};

#endif // PROJECT_MODEL_HPP
