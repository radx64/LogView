#include "UpdateChecker.hpp"

#include <algorithm>

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>

namespace
{
const auto kReleasesApiUrl =
    QStringLiteral("https://api.github.com/repos/%1/releases/latest")
        .arg(QStringLiteral(GITHUB_REPO));

// Splits a version string such as "v1.2.3" into its numeric components.
// Any leading 'v' is dropped and non-numeric suffixes (e.g. "1.2.0-rc1") are
// ignored on a per-component basis.
QList<int> parseVersion(const QString& raw)
{
    QString text = raw.trimmed();
    if (!text.isEmpty() && (text[0] == 'v' || text[0] == 'V'))
        text.remove(0, 1);

    QList<int> parts;
    const QStringList tokens = text.split('.', Qt::SkipEmptyParts);
    for (const QString& token : tokens)
    {
        QString digits;
        for (const QChar c : token)
        {
            if (c.isDigit())
                digits.append(c);
            else
                break;
        }
        parts.append(digits.isEmpty() ? 0 : digits.toInt());
    }
    return parts;
}
} // namespace

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent), manager_(new QNetworkAccessManager(this))
{
    connect(manager_, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::checkForUpdates()
{
    QNetworkRequest request{QUrl(kReleasesApiUrl)};
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("LogView/%1").arg(QString(APP_VERSION)));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    manager_->get(request);
}

bool UpdateChecker::isNewerVersion(const QString& latest, const QString& current)
{
    const QList<int> latestParts = parseVersion(latest);
    const QList<int> currentParts = parseVersion(current);

    const int count = std::max(latestParts.size(), currentParts.size());
    for (int i = 0; i < count; ++i)
    {
        const int l = i < latestParts.size() ? latestParts[i] : 0;
        const int c = i < currentParts.size() ? currentParts[i] : 0;
        if (l != c)
            return l > c;
    }
    return false;
}

namespace
{
// GitHub error responses carry a human-readable "message" field. Returns an
// empty string when the body is not JSON or has no message.
QString githubMessage(const QByteArray& payload)
{
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject())
        return {};
    return doc.object().value(QStringLiteral("message")).toString();
}

// Reads the rate-limit reset moment from the reply headers. GitHub uses an
// epoch timestamp ("X-RateLimit-Reset"); a relative "Retry-After" is honored as
// a fallback. Returns an invalid QDateTime if neither header is present.
QDateTime rateLimitReset(QNetworkReply* reply)
{
    if (reply->hasRawHeader("Retry-After"))
    {
        bool ok = false;
        const qint64 secs = reply->rawHeader("Retry-After").toLongLong(&ok);
        if (ok)
            return QDateTime::currentDateTime().addSecs(secs);
    }
    if (reply->hasRawHeader("X-RateLimit-Reset"))
    {
        bool ok = false;
        const qint64 epoch =
            reply->rawHeader("X-RateLimit-Reset").toLongLong(&ok);
        if (ok)
            return QDateTime::fromSecsSinceEpoch(epoch);
    }
    return {};
}
} // namespace

void UpdateChecker::onReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();

    const QByteArray payload = reply->readAll();
    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError)
    {
        // No release has been published for this repository yet.
        if (status == 404)
        {
            emit upToDate(QString(APP_VERSION));
            return;
        }

        if (status == 403 || status == 429)
        {
            const QString msg = githubMessage(payload);
            const bool rateLimited =
                msg.contains(QStringLiteral("rate limit"), Qt::CaseInsensitive) ||
                reply->rawHeader("X-RateLimit-Remaining") == "0" ||
                reply->hasRawHeader("Retry-After");

            if (rateLimited)
            {
                const QDateTime reset = rateLimitReset(reply);
                if (reset.isValid())
                {
                    const qint64 minutes =
                        (QDateTime::currentDateTime().secsTo(reset) + 59) / 60;
                    emit checkFailed(
                        minutes > 1
                            ? tr("GitHub API rate limit exceeded. Try again in "
                                 "about %1 minutes (at %2).")
                                  .arg(minutes)
                                  .arg(reset.toString(QStringLiteral("HH:mm")))
                            : tr("GitHub API rate limit exceeded. Try again at "
                                 "%1.")
                                  .arg(reset.toString(QStringLiteral("HH:mm"))));
                }
                else
                {
                    emit checkFailed(tr("GitHub API rate limit exceeded. "
                                        "Please try again later."));
                }
            }
            else
            {
                emit checkFailed(msg.isEmpty() ? reply->errorString() : msg);
            }
            return;
        }

        // Surface the HTTP status and GitHub's message when available; this is
        // far more useful than Qt's empty "server replied:" string.
        const QString msg = githubMessage(payload);
        if (status > 0)
            emit checkFailed(msg.isEmpty()
                                 ? tr("GitHub returned HTTP %1.").arg(status)
                                 : tr("GitHub returned HTTP %1: %2")
                                       .arg(status)
                                       .arg(msg));
        else
            emit checkFailed(reply->errorString());
        return;
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        emit checkFailed(tr("Could not parse update information from GitHub."));
        return;
    }

    const QJsonObject obj = doc.object();
    const QString tag = obj.value(QStringLiteral("tag_name")).toString();
    const QString url = obj.value(QStringLiteral("html_url")).toString();
    const QString title = obj.value(QStringLiteral("name")).toString();
    const QString notes = obj.value(QStringLiteral("body")).toString();

    if (tag.isEmpty())
    {
        emit checkFailed(tr("No release information is available yet."));
        return;
    }

    if (isNewerVersion(tag, QString(APP_VERSION)))
        emit updateAvailable(tag, url, title.isEmpty() ? tag : title, notes);
    else
        emit upToDate(QString(APP_VERSION));
}
