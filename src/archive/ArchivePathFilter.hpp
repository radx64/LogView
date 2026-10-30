#pragma once

#include <QString>
#include <QStringList>

namespace logview_archive
{

QStringList filterEntryPaths(const QStringList& paths, const QString& pattern);

}  // namespace logview_archive
