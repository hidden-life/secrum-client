#ifndef CORE_CRYPTO_RATCHET_STORE_H
#define CORE_CRYPTO_RATCHET_STORE_H

#include <QJsonObject>
#include <QString>

class RatchetStore {
public:
    static QString keyFor(const QString &peerUserId, const QString &peerDeviceId) {
        return "ratchet:" + peerUserId + ":" + peerDeviceId;
    }

    static void save(const QString &peerUserId, const QString &peerDeviceId, const QJsonObject &state);
    static QJsonObject load(const QString &peerUserId, const QString &peerDeviceId);
    static void remove(const QString &peerUserId, const QString &peerDeviceId);
};

#endif //CORE_CRYPTO_RATCHET_STORE_H
