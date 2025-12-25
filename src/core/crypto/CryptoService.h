#ifndef CORE_CRYPTO_CRYPTO_SERVICE_H
#define CORE_CRYPTO_CRYPTO_SERVICE_H

#include <QObject>

struct EncryptResult {
    QString cipherTextB64;
    QString pubKeyB64;
    std::optional<QString> x3dhOtpId;
};

class CryptoService final : public QObject{
    Q_OBJECT
public:
    static CryptoService &instance();

    bool init();

    bool ensureSession(const QString &peerUserId, const QString &peerDeviceId, const QJsonObject &bundleJson);

    EncryptResult encryptToDevice(const QString &peerUserId, const QString &peerDeviceId, const QString &plainText);
    QString decryptFromDevice(const QString &peerUserId, const QString &peerDeviceId, const QString &senderPubKeyB64, const QString &cipherTextB64);

private:
    bool m_isReady = false;
    QString m_identitySKB64;
    QString m_identityPKB64;

    bool loadOrCreateIdentity();

    explicit CryptoService(QObject *parent = nullptr);
};

#endif //CORE_CRYPTO_CRYPTO_SERVICE_H
