#pragma once

#include <memory>
#include <vector>
#include <QString>
#include <QObject>

#include "Logfile.hpp"

namespace serializer { class ProjectModel; class ArchiveDocument; }

class ArchiveDocument;

class ProjectModel : public QObject
{
Q_OBJECT
public:
    ProjectModel();
    virtual ~ProjectModel();

    Logfile* add_to_project(std::unique_ptr<Logfile>&& lf);
    ArchiveDocument* add_archive_to_project(std::unique_ptr<ArchiveDocument>&& archive);
    std::vector<std::unique_ptr<Logfile>>& get_log_files();
    std::vector<std::unique_ptr<ArchiveDocument>>& get_archives();
    bool is_empty();
    void remove_file_from_project(Logfile* logfile);
    void remove_archive_from_project(ArchiveDocument* archive);

    const QString& name() const;
    void setName(const QString& name);

    bool isDirty() const;
    void markDirty();
    void markClean();

protected:
    friend class serializer::ProjectModel;
    friend class serializer::ArchiveDocument;
    std::vector<std::unique_ptr<Logfile>> logfiles_;
    std::vector<std::unique_ptr<ArchiveDocument>> archives_;
    QString projectName_;
    bool dirty_;

protected slots:
    void on_logfile_change();

signals:
    void stateChanged();
};
