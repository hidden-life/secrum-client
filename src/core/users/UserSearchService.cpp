#include "UserSearchService.h"

#include <QJsonDocument>
#include <QUrlQuery>

UserSearchService::UserSearchService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        QVector<UserSearchResult> results;

        if (doc.isObject()) {
            const auto obj = doc.object();
            UserSearchResult r;
            r.userId = obj["user_id"].toString();
            r.displayName = obj["display_name"].toString();
            r.username = obj["username"].isNull() ? QString() : obj["username"].toString();

            results.push_back(r);
        }

        emit searchCompleted(results);
    });

    connect(m_httpClient, &HttpClient::error, this, &UserSearchService::searchFailed);
}

void UserSearchService::search(const QString &query) {
    const QString q = query.trimmed();
    if (q.isEmpty()) {
        emit searchCompleted({});
        return;
    }

    QUrl url;
    url.setPath("/users/search");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", q);
    url.setQuery(urlQuery);

    const QString path = QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));

    m_httpClient->get(path);
}
