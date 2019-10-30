#ifndef PROJECT_MODEL_HPP
#define PROJECT_MODEL_HPP

#include <memory>
#include <vector>

class Logfile;

namespace serializer { class Logfile; }

class ProjectModel
{
public:
    ProjectModel() = default;
    std::vector<std::unique_ptr<Logfile>> logfiles_;
protected:
    friend class serializer::Logfile;
};

#endif // PROJECT_MODEL_HPP
