#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

// Queries the GitHub Releases API for the latest published release and compares
// it against the locally built version. All network work is asynchronous so the
// UI thread is never blocked.
class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject* parent = nullptr);

    void checkForUpdates();

    // Returns true if "latest" represents a strictly newer version than
    // "current". Tags may be prefixed with 'v' (e.g. "v0.1.0").
    static bool isNewerVersion(const QString& latest, const QString& current);

signals:
    void updateAvailable(const QString& version,
                         const QString& url,
                         const QString& title,
                         const QString& notes);
    void upToDate(const QString& currentVersion);
    void checkFailed(const QString& error);

private:
    void onReplyFinished(QNetworkReply* reply);

    QNetworkAccessManager* manager_{nullptr};
};
