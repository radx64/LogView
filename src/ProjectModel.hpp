#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <vector>
#include <QString>
#include <QObject>

#include "Logfile.hpp"

namespace serializer { class Logfile; }

class ProjectModel : public QObject
{
Q_OBJECT
public:
    ProjectModel();
    virtual ~ProjectModel() = default;
    std::vector<std::unique_ptr<Logfile>> logfiles_;

    QString projectName_;
    bool changed_;

    void mocked_change();

protected:
    friend class serializer::Logfile;

signals:
    void changed();
};

#endif // PROJECT_MODEL_HPP
