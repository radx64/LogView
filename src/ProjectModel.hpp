#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <vector>
#include <QString>
#include <QObject>

#include "Logfile.hpp"

namespace serializer { class ProjectModel; }

class ProjectModel : public QObject
{
Q_OBJECT
public:
    ProjectModel();
    virtual ~ProjectModel();

    Logfile* add_to_project(std::unique_ptr<Logfile>&& lf);
    std::vector<std::unique_ptr<Logfile>>& get_log_files();
    bool is_empty();
    void remove_file_from_project(Logfile* logfile);
    QString projectName_;
    bool changed_;

protected:
    friend class serializer::ProjectModel;
    std::vector<std::unique_ptr<Logfile>> logfiles_;

protected slots:
    void on_logfile_change();

signals:
    void changed(); // is emitted whenever project is changed
};

#endif // PROJECT_MODEL_HPP
