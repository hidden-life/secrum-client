#include "UserSearchService.h"

#include <QJsonDocument>

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
    if (query.trimmed().isEmpty()) {
        emit searchCompleted({});
        return;
    }

    m_httpClient->get("/users/search?q=" + query);
}
