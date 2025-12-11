#ifndef CORE_CRYPTO_CRYPTO_SERVICE_H
#define CORE_CRYPTO_CRYPTO_SERVICE_H

#include <QObject>

class CryptoService final : public QObject{
    Q_OBJECT
public:
    static CryptoService &instance();

    bool init();

    QString encryptForChat(const QString &id, const QString &peerUserId, const QString &plainText);
    QString decryptForChat(const QString &id, const QString &peerUserId, const QString &cipherText);

private:
    bool m_isReady = false;

    explicit CryptoService(QObject *parent = nullptr);
    QByteArray deriveKey(const QString &id, const QString &peerUserId) const;
};

#endif //CORE_CRYPTO_CRYPTO_SERVICE_H
