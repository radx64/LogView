#include "ProjectModel.hpp"

ProjectModel::ProjectModel() : projectName_("<empty>"), changed_{false}
{
}

void ProjectModel::mocked_change()
{
    //TODO: This should be detectable by changing project model
    emit changed();
}
