#include "CryptoService.h"

#include <QByteArray>
#include <QDebug>
#include <sodium.h>

#include "RatchetStore.h"
#include "core/storage/SQLiteStorage.h"

// XChaCha20-Poly1305 sizes
static constexpr size_t KEY_BYTES = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
static constexpr size_t NONCE_BYTES = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
// version of encrypted message
static constexpr unsigned char CURRENT_VERSION = 1;

static QByteArray b64decode(const QString &str) {
    QByteArray out(str.size(), '\0');
    size_t outLength = 0;
    if (str.isEmpty()) return {};
    if (sodium_base642bin((unsigned char*)out.data(), out.size(), str.toUtf8().constData(), str.size(), nullptr, &outLength, nullptr, sodium_base64_VARIANT_ORIGINAL) != 0) {
        return {};
    }
    out.resize(static_cast<int>(outLength));

    return out;
}

static QString b64encode(const QByteArray &bin) {
    if (bin.isEmpty()) return {};
    const int maxLength = sodium_base64_ENCODED_LEN(bin.size(), sodium_base64_VARIANT_ORIGINAL);
    QByteArray out(maxLength, '\0');
    sodium_bin2base64(out.data(), out.size(), (const unsigned char*)bin.constData(), bin.size(), sodium_base64_VARIANT_ORIGINAL);

    return QString::fromUtf8(out.constData());
}

static QByteArray hkdfExtract(const QByteArray &salt, const QByteArray &ikm) {
    unsigned char prk[crypto_auth_hmacsha256_BYTES];
    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, (const unsigned char*)salt.constData(), (size_t)salt.size());
    crypto_auth_hmacsha256_update(&st, (const unsigned char*)ikm.constData(), (size_t)ikm.size());
    crypto_auth_hmacsha256_final(&st, prk);

    return QByteArray((char*)prk, crypto_auth_hmacsha256_BYTES);
}

static QByteArray hkdfExpand(const QByteArray &prk, const QByteArray &info, int length) {
    QByteArray okm;
    okm.reserve(length);

    QByteArray t;
    unsigned char out[crypto_auth_hmacsha256_BYTES];

    int counter = 1;
    while (okm.size() < length) {
        crypto_auth_hmacsha256_state st;
        crypto_auth_hmacsha256_init(&st, (const unsigned char*)prk.constData(), (size_t)prk.size());
        if (!t.isEmpty()) crypto_auth_hmacsha256_update(&st, (const unsigned char*)t.constData(), t.size());
        if (!info.isEmpty()) crypto_auth_hmacsha256_update(&st, (const unsigned char*)info.constData(), info.size());

        unsigned char c = (unsigned char)counter;
        crypto_auth_hmacsha256_update(&st, &c, 1);
        crypto_auth_hmacsha256_final(&st, out);

        t = QByteArray((char*)out, crypto_auth_hmacsha256_BYTES);

        const int need = std::min<long long>(length - okm.size(), (int)t.size());
        okm.append(t.left(need));

        counter++;
    }

    return okm;
}

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

    return loadOrCreateIdentity();
}

bool CryptoService::ensureSession(const QString &peerUserId, const QString &peerDeviceId, const QJsonObject &bundleJson) {
    // if already exists
    const QJsonObject existing = RatchetStore::load(peerUserId, peerDeviceId);
    if (!existing.isEmpty()) {
        return true;
    }

    const QString ikB64 = bundleJson.value("identity_key").toString();
    const QString spkB64 = bundleJson.value("signed_prekey").toString();

    if (ikB64.isEmpty() || spkB64.isEmpty()) {
        return false;
    }

    const QByteArray IKr = b64decode(ikB64);
    const QByteArray SPKr = b64decode(spkB64);

    // optional OTPK
    std::optional<QString> otpkId;
    QByteArray OTPKr;
    if (bundleJson.contains("one_time_prekey") && bundleJson["one_time_prekey"].isObject()) {
        const auto o = bundleJson.value("one_time_prekey").toObject();
        const QString id = o.value("id").toString();
        const QString pub = o.value("public_key").toString();
        if (!id.isEmpty() && !pub.isEmpty()) {
            otpkId = id;
            OTPKr = b64decode(pub);
        }
    }

    // load our identity
    const QByteArray IKs_sk = b64decode(m_identitySKB64);
    const QByteArray IKs_pk = b64decode(m_identityPKB64);

    // create ephemeral EKs
    unsigned char EKs_pk[crypto_kx_PUBLICKEYBYTES];
    unsigned char EKs_sk[crypto_kx_SECRETKEYBYTES];

    crypto_kx_keypair(EKs_pk, EKs_sk);

    auto dh = [](const QByteArray &sk, const QByteArray &pk) -> QByteArray {
        unsigned char out[crypto_scalarmult_BYTES];
        if (sk.size() != crypto_scalarmult_SCALARBYTES || pk.size() != crypto_scalarmult_BYTES) {
            return {};
        }

        if (crypto_scalarmult(out, (const unsigned char*)sk.constData(), (const unsigned char*)pk.constData()) != 0) return {};

        return QByteArray((char*)out, sizeof(out));
    };

    QByteArray dh1 = dh(IKs_sk, SPKr);
    QByteArray dh2 = dh(QByteArray((char*)EKs_sk, crypto_scalarmult_SCALARBYTES), IKr);
    QByteArray dh3 = dh(QByteArray((char*)EKs_sk, crypto_scalarmult_SCALARBYTES), SPKr);

    if (dh1.isEmpty() || dh2.isEmpty() || dh3.isEmpty()) {
        return false;
    }

    QByteArray ikm = dh1 + dh2 + dh3;

    if (!OTPKr.isEmpty()) {
        QByteArray dh4 = dh(QByteArray((char*)EKs_sk, crypto_scalarmult_SCALARBYTES), OTPKr);
        if (dh4.isEmpty()) {
            return false;
        }

        ikm += dh4;
    }

    // derive initial root key + chain key
    QByteArray salt(32, '\0'); // all zero
    const QByteArray prk = hkdfExtract(salt, ikm);
    const QByteArray okm = hkdfExpand(prk, QByteArray("Secrum-X3DH", 10), 64);

    QByteArray RK = okm.left(32);
    QByteArray CKs = okm.mid(32, 32);

    // init ratchet
    const QString DHs_pk_b64 = b64encode(QByteArray((char*)EKs_pk, crypto_kx_PUBLICKEYBYTES));
    const QString DHs_sk_b64 = b64encode(QByteArray((char*)EKs_sk, crypto_kx_SECRETKEYBYTES));

    QJsonObject st;
    st["rk"] = b64encode(RK);
    st["cks"] = b64encode(CKs);
    st["ckr"] = "";
    st["dhs_pk"] = DHs_pk_b64;
    st["dhs_sk"] = DHs_sk_b64;
    st["dhr_pk"] = "";
    st["ns"] = 0;
    st["nr"] = 0;
    st["pn"] = 0;

    if (otpkId.has_value()) st["x3dh_otpk_id"] = otpkId.value();

    RatchetStore::save(peerUserId, peerDeviceId, st);
    return true;
}

EncryptResult CryptoService::encryptToDevice(const QString &peerUserId, const QString &peerDeviceId, const QString &plainText) {
    QJsonObject st = RatchetStore::load(peerUserId, peerDeviceId);
    if (st.isEmpty()) {
        return {};
    }

    QByteArray CKs = b64decode(st.value("cks").toString());
    if (CKs.isEmpty()) return {};

    // Derive mk and next CKs via HKDF(CKs, "DR-message", 64)
    const QByteArray prk = hkdfExtract(QByteArray(32, '\0'), CKs);
    const QByteArray okm = hkdfExpand(prk, QByteArray("Secrum-DR-msg", 13), 64);

    QByteArray mk = okm.left(32);
    QByteArray nextCKs = okm.mid(32, 32);

    // Update Ns
    int ns = st.value("ns").toInt();
    st["ns"] = ns + 1;
    st["cks"] = b64encode(nextCKs);

    // AEAD encrypt
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    QByteArray aad; // keep simple for now
    aad.append(peerUserId.toUtf8());
    aad.append("|");
    aad.append(peerDeviceId.toUtf8());
    aad.append("|");
    aad.append(QString::number(ns).toUtf8());

    QByteArray pt = plainText.toUtf8();
    QByteArray ct(pt.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES, '\0');
    unsigned long long ctLen = 0;

    crypto_aead_xchacha20poly1305_ietf_encrypt(
        (unsigned char*)ct.data(), &ctLen,
        (const unsigned char*)pt.constData(), pt.size(),
        (const unsigned char*)aad.constData(), aad.size(),
        nullptr,
        nonce,
        (const unsigned char*)mk.constData()
    );
    ct.resize((int)ctLen);

    // Pack nonce||ct
    QByteArray packed((char*)nonce, sizeof(nonce));
    packed.append(ct);

    RatchetStore::save(peerUserId, peerDeviceId, st);

    EncryptResult res;
    res.cipherTextB64 = b64encode(packed);
    res.pubKeyB64 = st.value("dhs_pk").toString();

    if (st.contains("x3dh_otpk_id")) {
        res.x3dhOtpId = st.value("x3dh_otpk_id").toString();
    }

    return res;
}

QString CryptoService::decryptFromDevice(const QString &peerUserId, const QString &peerDeviceId, const QString &senderPubKeyB64, const QString &cipherTextB64) {
    QJsonObject st = RatchetStore::load(peerUserId, peerDeviceId);
    if (st.isEmpty()) return {};

    const QString currentDHr = st.value("dhr_pk").toString();
    if (currentDHr != senderPubKeyB64) {
        // DH ratchet step
        QByteArray RK = b64decode(st.value("rk").toString());
        QByteArray DHs_sk = b64decode(st.value("dhs_sk").toString());
        QByteArray DHr_pk = b64decode(senderPubKeyB64);

        // DH = scalarmult(DHs_sk, DHr_pk)
        unsigned char dhOut[crypto_scalarmult_BYTES];
        if (DHs_sk.size() != crypto_scalarmult_SCALARBYTES || DHr_pk.size() != crypto_scalarmult_BYTES) return {};
        if (crypto_scalarmult(dhOut,
                              (const unsigned char*)DHs_sk.constData(),
                              (const unsigned char*)DHr_pk.constData()) != 0) return {};
        QByteArray dh = QByteArray((char*)dhOut, sizeof(dhOut));

        // Derive new RK and CKr
        const QByteArray prk = hkdfExtract(RK, dh);
        const QByteArray okm = hkdfExpand(prk, QByteArray("Secrum-DR-ratchet", 18), 64);

        QByteArray newRK = okm.left(32);
        QByteArray CKr = okm.mid(32, 32);

        st["rk"] = b64encode(newRK);
        st["ckr"] = b64encode(CKr);
        st["dhr_pk"] = senderPubKeyB64;

        // reset counters
        st["pn"] = st.value("ns").toInt();
        st["ns"] = 0;
        st["nr"] = 0;

        // also rotate our DHs (new sending ratchet key)
        unsigned char newDHs_pk[crypto_kx_PUBLICKEYBYTES];
        unsigned char newDHs_sk[crypto_kx_SECRETKEYBYTES];
        crypto_kx_keypair(newDHs_pk, newDHs_sk);

        st["dhs_pk"] = b64encode(QByteArray((char*)newDHs_pk, sizeof(newDHs_pk)));
        st["dhs_sk"] = b64encode(QByteArray((char*)newDHs_sk, sizeof(newDHs_sk)));

        // derive CKs from DH(new DHs, DHr) for next sends (как в DR)
        unsigned char dhOut2[crypto_scalarmult_BYTES];
        if (crypto_scalarmult(dhOut2, newDHs_sk, (const unsigned char*)DHr_pk.constData()) != 0) return {};
        QByteArray dh2((char*)dhOut2, sizeof(dhOut2));

        const QByteArray prk2 = hkdfExtract(newRK, dh2);
        const QByteArray okm2 = hkdfExpand(prk2, QByteArray("Secrum-DR-send", 14), 64);
        st["cks"] = b64encode(okm2.mid(32, 32)); // CKs
    }

    QByteArray CKr = b64decode(st.value("ckr").toString());
    if (CKr.isEmpty()) return {}; // если пусто — значит нет входящей цепочки

    // unpack nonce||ct
    QByteArray packed = b64decode(cipherTextB64);
    if (packed.size() < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES) return {};
    const QByteArray nonce = packed.left(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    const QByteArray ct = packed.mid(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);

    // mk and next CKr
    const QByteArray prk = hkdfExtract(QByteArray(32, '\0'), CKr);
    const QByteArray okm = hkdfExpand(prk, QByteArray("Secrum-DR-msg", 13), 64);
    QByteArray mk = okm.left(32);
    QByteArray nextCKr = okm.mid(32, 32);

    int nr = st.value("nr").toInt();

    QByteArray aad;
    aad.append(peerUserId.toUtf8());
    aad.append("|");
    aad.append(peerDeviceId.toUtf8());
    aad.append("|");
    aad.append(QString::number(nr).toUtf8());

    QByteArray pt(ct.size(), '\0');
    unsigned long long ptLen = 0;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            (unsigned char*)pt.data(), &ptLen,
            nullptr,
            (const unsigned char*)ct.constData(), ct.size(),
            (const unsigned char*)aad.constData(), aad.size(),
            (const unsigned char*)nonce.constData(),
            (const unsigned char*)mk.constData()) != 0) {
        return {};
    }

    pt.resize((int)ptLen);

    st["nr"] = nr + 1;
    st["ckr"] = b64encode(nextCKr);

    RatchetStore::save(peerUserId, peerDeviceId, st);
    return QString::fromUtf8(pt);
}

bool CryptoService::loadOrCreateIdentity() {
    auto &st = SQLiteStorage::instance();
    m_identitySKB64 = st.get("crypto:ik:sk");
    m_identityPKB64 = st.get("crypto:ik:pk");

    if (!m_identitySKB64.isEmpty() && !m_identityPKB64.isEmpty()) {
        return true;
    }

    unsigned char pk[crypto_kx_PUBLICKEYBYTES];
    unsigned char sk[crypto_kx_SECRETKEYBYTES];
    crypto_kx_keypair(pk, sk);

    m_identityPKB64 = b64encode(QByteArray((char*)pk, sizeof(pk)));
    m_identitySKB64 = b64encode(QByteArray((char*)sk, sizeof(sk)));

    st.set("crypto:ik:sk", m_identitySKB64);
    st.set("crypto:ik:pk", m_identityPKB64);

    m_isReady = true;
    qDebug() << "[CRYPTO] initialized. Load identity.";

    return true;
}

CryptoService::CryptoService(QObject *parent) : QObject(parent) {
}
