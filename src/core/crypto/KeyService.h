#ifndef CORE_CRYPTO_KEY_SERVICE_H
#define CORE_CRYPTO_KEY_SERVICE_H

#include <QObject>

#include "core/network/HttpClient.h"

class KeyService final : public  QObject{
    Q_OBJECT
public:
    static KeyService &instance();

    void fetchBundle(const QString &userId);

signals:
    void bundleLoaded(const QString &userId, const QJsonObject &bundle);
    void bundleFailed(const QString &userId, const QString &error);

private:
    HttpClient *m_httpClient;

    explicit KeyService(QObject *parent = nullptr);
};

#endif //CORE_CRYPTO_KEY_SERVICE_H
