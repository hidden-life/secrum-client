#ifndef CORE_USERS_USER_SEARCH_SERVICE_H
#define CORE_USERS_USER_SEARCH_SERVICE_H

#include <QObject>
#include <QVector>

#include "core/model/UserSearchResult.h"
#include "core/network/HttpClient.h"

class UserSearchService : public QObject {
    Q_OBJECT
public:
    explicit UserSearchService(QObject *parent = nullptr);
    void search(const QString &query);

signals:
    void searchCompleted(const QVector<UserSearchResult> &result);
    void searchFailed(const QString &error);

private:
    HttpClient *m_httpClient;
};

#endif //CORE_USERS_USER_SEARCH_SERVICE_H
