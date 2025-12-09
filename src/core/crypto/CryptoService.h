#ifndef CORE_CRYPTO_CRYPTO_SERVICE_H
#define CORE_CRYPTO_CRYPTO_SERVICE_H

#include <QObject>

class CryptoService final : public QObject{
    Q_OBJECT
public:
    static CryptoService &instance();

    void init();
    bool isReady() const;

    QString encryptForChat(const QString &peerUserId, const QString &plainText);
    QString decryptForChat(const QString &peerUserId, const QString &cipherText);

private:
    explicit CryptoService(QObject *parent = nullptr);
    CryptoService(const CryptoService &) = delete;
    CryptoService &operator=(const CryptoService &) = delete;

    QByteArray m_masterKey;
    bool m_initialized = false;

    void loadOrCreateMasterKey();
};

#endif //CORE_CRYPTO_CRYPTO_SERVICE_H
