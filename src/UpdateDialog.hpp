#pragma once

#include <QDialog>
#include <QString>

// Shown when a newer release is found. Displays version information, renders the
// release notes (GitHub markdown) and offers actions to open the download page
// or skip the version.
class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    UpdateDialog(const QString& currentVersion,
                 const QString& latestVersion,
                 const QString& url,
                 const QString& title,
                 const QString& notes,
                 QWidget* parent = nullptr);

signals:
    void skipVersionRequested(const QString& version);

private:
    QString url_;
    QString latestVersion_;
};
