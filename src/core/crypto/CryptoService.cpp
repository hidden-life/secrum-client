#include "CryptoService.h"

#include "core/storage/SQLiteStorage.h"
#include <QByteArray>
#include <QRandomGenerator>

CryptoService &CryptoService::instance() {
    static CryptoService instance;
    return instance;
}

CryptoService::CryptoService(QObject *parent) : QObject(parent) {
}

void CryptoService::init() {
    if (m_initialized) {
        return;
    }

    loadOrCreateMasterKey();
    m_initialized = true;
}

bool CryptoService::isReady() const {
    return m_initialized && !m_masterKey.isEmpty();
}

void CryptoService::loadOrCreateMasterKey() {
    auto &storage = SQLiteStorage::instance();
    const QString stored = storage.get("crypto_master_key");

    if (!stored.isEmpty()) {
        const QByteArray key = QByteArray::fromBase64(stored.toUtf8());
        if (key.size() == 32) {
            m_masterKey = key;
            qDebug() << "[CRYPTO] Master key loaded from storage.";
            return;
        } else {
            qWarning() << "[CRYPTO] Invalid master key size in storage, regenrating.";
        }

        QByteArray k(32, 0);
        for (int i = 0; i < k.size(); ++i) {
            k[i] = static_cast<char>(QRandomGenerator::global()->generate() & 0xFF);
        }

        storage.set("crypto_master_key", QString::fromUtf8(k.toBase64()));
        m_masterKey = k;
        qDebug() << "[CRYPTO] Generated new master key.";
    }
}

QString CryptoService::encryptForChat(const QString &peerUserId, const QString &plainText) {
    Q_UNUSED(peerUserId);

    const QByteArray data = plainText.toUtf8();
    const QByteArray wrapped = "v0:" + data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    return QString::fromUtf8(wrapped);
}

QString CryptoService::decryptForChat(const QString &peerUserId, const QString &cipherText) {
    Q_UNUSED(peerUserId);

    if (cipherText.startsWith("v0:")) {
        const QByteArray b64 = cipherText.mid(3).toUtf8();
        const QByteArray decoded = QByteArray::fromBase64(b64, QByteArray::Base64UrlEncoding);
        if (!decoded.isEmpty()) {
            return QString::fromUtf8(decoded);
        }
    }

    return cipherText;
}
