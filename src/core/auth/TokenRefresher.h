#ifndef CORE_AUTH_TOKEN_REFRESHER_H
#define CORE_AUTH_TOKEN_REFRESHER_H

#include <QObject>

class HttpClient;

class TokenRefresher final : public QObject {
    Q_OBJECT
public:
    explicit TokenRefresher(HttpClient *client, QObject *parent = nullptr);
    void refresh();

signals:
    void refreshSuccess();
    void refreshFailed(const QString &error);

private:
    HttpClient *m_httpClient = nullptr;
    bool m_refreshInProgress = false;
};

#endif //CORE_AUTH_TOKEN_REFRESHER_H
