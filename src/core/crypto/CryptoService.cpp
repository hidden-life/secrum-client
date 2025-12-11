#include "CryptoService.h"

#include <QByteArray>
#include <QDebug>
#include <sodium.h>

// XChaCha20-Poly1305 sizes
static constexpr size_t KEY_BYTES = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
static constexpr size_t NONCE_BYTES = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
// version of encrypted message
static constexpr unsigned char CURRENT_VERSION = 1;

static const unsigned char APP_SECRET_KEY[crypto_generichash_KEYBYTES] = {
    0x53, 0x65, 0x63, 0x72, 0x75, 0x6d, 0x21, 0x21,
    0x01, 0x02, 0x03, 0xA5, 0x7B, 0x99, 0x42, 0x10,
    0xFF, 0xEE, 0xDD, 0xCC, 0xBA, 0xBE, 0x13, 0x37,
    0xDE, 0xAD, 0xBE, 0xEF, 0x21, 0x22, 0x23, 0x24,
};

CryptoService &CryptoService::instance() {
    static CryptoService instance;
    return instance;
}

bool CryptoService::init() {
    if (m_isReady) {
        return true;
    }

    if (sodium_init() < 0) {
        qWarning() << "[CRYPTO] sodium_init failed.";
        m_isReady = false;
        return false;
    }

    m_isReady = true;
    qDebug() << "[CRYPTO] initialized.";
    return true;
}

QString CryptoService::encryptForChat(const QString &id, const QString &peerUserId, const QString &plainText) {
    if (!m_isReady) {
        qWarning() << "[CRYPTO] not initialized, returning plainText.";
        return plainText;
    }

    const QByteArray key = deriveKey(id, peerUserId);
    const QByteArray msg = plainText.toUtf8();

    // AAD connect message with all members of chat
    const QString a = id < peerUserId ? id : peerUserId;
    const QString b = id < peerUserId ? peerUserId : id;
    const QByteArray aad = (a + "|" + b).toUtf8();

    unsigned char nonce[NONCE_BYTES];
    randombytes_buf(nonce, sizeof nonce);

    QByteArray cipher(msg.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES, 0);
    unsigned long long cipherLength = 0;

    int rc = crypto_aead_xchacha20poly1305_ietf_encrypt(
        reinterpret_cast<unsigned char *>(cipher.data()),
        &cipherLength,
        reinterpret_cast<const unsigned char *>(msg.constData()),
        static_cast<unsigned long long>(msg.size()),
        reinterpret_cast<const unsigned char *>(aad.constData()),
        static_cast<unsigned long long>(aad.size()),
        nullptr,
        nonce,
        reinterpret_cast<const unsigned char *>(key.constData())
    );

    if (rc != 0) {
        qWarning() << "[CRYPTO] encrypt failed., returning plaintext.";
        return plainText;
    }

    cipher.resize(static_cast<int>(cipherLength));
    // format: [1 byte version][NONCE][CIPHER TEXT...]
    QByteArray packed;
    packed.reserve(1 + NONCE_BYTES + cipher.size());
    packed.append(static_cast<char>(CURRENT_VERSION));
    packed.append(reinterpret_cast<const char *>(nonce), NONCE_BYTES);
    packed.append(cipher);

    return QString::fromLatin1(packed.toBase64());
}

QString CryptoService::decryptForChat(const QString &id, const QString &peerUserId, const QString &cipherText) {
    if (!m_isReady) {
        qWarning() << "[CRYPTO] not initialized, returning raw cipher text.";
        return cipherText;
    }

    if (cipherText.isEmpty()) {
        return {};
    }

    QByteArray packed = QByteArray::fromBase64(cipherText.toLatin1());
    if (packed.size() < 1 + static_cast<int>(NONCE_BYTES)) {
        return QString::fromUtf8(packed);
    }

    const unsigned char version = static_cast<unsigned char>(packed[0]);
    if (version != CURRENT_VERSION) {
        return QStringLiteral("[Unsupported encrypted message]");
    }

    const unsigned char *nonce = reinterpret_cast<const unsigned char *>(packed.constData() + 1);
    const int cipherLength = packed.size() - 1 - static_cast<int>(NONCE_BYTES);
    if (cipherLength <= 0) {
        return {};
    }

    const unsigned char *cipherPtr = reinterpret_cast<const unsigned char *>(packed.constData() + 1 + NONCE_BYTES);
    QByteArray key = deriveKey(id, peerUserId);

    const QString a = id < peerUserId ? id : peerUserId;
    const QString b = id < peerUserId ? peerUserId : id;
    const QByteArray aad = (a + "|" + b).toUtf8();
    QByteArray plain(cipherLength, 0);

    unsigned long long plainLength = 0;

    int rc = crypto_aead_xchacha20poly1305_ietf_decrypt(
        reinterpret_cast<unsigned char *>(plain.data()),
        &plainLength,
        nullptr,
        cipherPtr,
        static_cast<unsigned long long>(cipherLength),
        reinterpret_cast<const unsigned char *>(aad.constData()),
        static_cast<unsigned long long>(aad.size()),
        nonce,
        reinterpret_cast<const unsigned char *>(key.constData())
    );

    if (rc != 0) {
        qWarning() << "[CRYPTO] decrypt failed.";
        return QStringLiteral("[Decryption failed]");
    }

    plain.resize(static_cast<int>(plainLength));
    return QString::fromUtf8(plain);
}

CryptoService::CryptoService(QObject *parent) : QObject(parent) {
}

QByteArray CryptoService::deriveKey(const QString &id, const QString &peerUserId) const {
    QByteArray key(KEY_BYTES, 0);

    // symmetric (min,max)
    const QString a = id < peerUserId ? id : peerUserId;
    const QString b = id < peerUserId ? peerUserId : id;

    if (const QByteArray input = a.toUtf8() + "|" + b.toUtf8(); crypto_generichash(
                                                              reinterpret_cast<unsigned char *>(key.data()),
                                                              key.size(),
                                                              reinterpret_cast<const unsigned char *>(input.constData()),
                                                              static_cast<unsigned long long>(input.size()),
                                                              APP_SECRET_KEY,
                                                              sizeof(APP_SECRET_KEY)) != 0) {
        qWarning() << "[CRYPTO] crypto_generichash failed.";
        key.fill(0);
    }

    return key;
}
