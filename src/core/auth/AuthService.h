#ifndef CORE_AUTH_AUTH_SERVICE_H
#define CORE_AUTH_AUTH_SERVICE_H

#include <QObject>

#include "core/network/HttpClient.h"

class AuthService final : public QObject {
Q_OBJECT
public:
    explicit AuthService(QObject *parent = nullptr);

    void loginBegin(const QString &phone);
    void verifyCode(const QString &code);

signals:
    void loginCodeSent();
    void loginSuccess(QString accessToken);
    void loginError(QString msg);

private:
    HttpClient *m_httpClient;
    QString m_requestId;
};

#endif //CORE_AUTH_AUTH_SERVICE_H
