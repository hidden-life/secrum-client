#include "MessageService.h"

#include <QJsonArray>

#include "core/auth/AuthSession.h"
#include "core/crypto/CryptoService.h"
#include "core/crypto/KeyService.h"

MessageService::MessageService(WSClient *wsClient, QObject *parent) :
        QObject(parent),
        m_wsClient(wsClient),
        m_httpClient(new HttpClient(this)),
        m_historyHttpClient(new HttpClient(this)) {
    m_httpClient->setAccessToken(AuthSession::instance().accessToken());
    m_historyHttpClient->setAccessToken(AuthSession::instance().accessToken());

    connect(wsClient, &WSClient::eventReceived, this, &MessageService::handleIncoming);

    connect(m_httpClient, &HttpClient::success, this, &MessageService::onHttpSuccess);
    connect(m_httpClient, &HttpClient::error, this, &MessageService::onHttpError);

    connect(m_historyHttpClient, &HttpClient::success, this, &MessageService::onHistoryHttpSuccess);
    connect(m_historyHttpClient, &HttpClient::error, this, &MessageService::onHistoryHttpError);

    connect(&KeyService::instance(), &KeyService::bundleLoaded, this, [this](const QString &userId, const QJsonObject &bundle) {
        if (userId != m_pendingPeerUserId) return;

        const QJsonArray devices = bundle.value("devices").toArray();
        if (devices.isEmpty()) {
            emit messageFailed(m_pendingMessageId, "No devices in bundle.");
            return;
        }

        const QJsonObject dev = devices.first().toObject();
        const QString deviceId = dev.value("device_id").toString();
        if (deviceId.isEmpty()) {
            emit messageFailed(m_pendingMessageId, "Bundle device_id is missing");
            return;
        }

        m_pendingPeerDeviceId = deviceId;

        if (!CryptoService::instance().ensureSession(m_pendingPeerUserId, deviceId, dev)) {
            emit messageFailed(m_pendingMessageId, "Failed to init crypto session");
            return;
        }

        if (m_pendingAfterBundle) {
            m_pendingAfterBundle();
            m_pendingAfterBundle = nullptr;
        }
    }, Qt::QueuedConnection);

    connect(&KeyService::instance(), &KeyService::bundleFailed, this, [this](const QString &userId, const QString &err) {
        if (userId == m_pendingPeerUserId) {
            emit messageFailed(m_pendingMessageId, "Bundle failed: " + err);
        }
    }, Qt::QueuedConnection);
}

void MessageService::sendMessage(const QString &peerUserId, const QString &peerDeviceId, const QString &text) {
    if (peerUserId.isEmpty() || text.isEmpty()) {
        return;
    }

    // create local message
    Message msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.peerUserId = peerUserId;
    msg.plainText = text;
    msg.isOutgoing = true;
    msg.status = MessageStatus::Sending;
    msg.createdAt = QDateTime::currentDateTimeUtc();

    m_messages[msg.id] = msg;
    emit messageAdded(msg);

    // save pending-context
    m_pendingMessageId = msg.id;
    m_pendingPeerUserId = peerUserId;
    m_pendingPeerDeviceId = peerDeviceId;
    m_pendingPlainText = text;

    auto encryptAndSend = [this]() {
        if (m_pendingPeerDeviceId.isEmpty()) {
            emit messageFailed(m_pendingMessageId, "peer device id is empty");
            return;
        }

        EncryptResult enc = CryptoService::instance().encryptToDevice(m_pendingPeerUserId, m_pendingPeerDeviceId, m_pendingPlainText);
        if (enc.cipherTextB64.isEmpty() || enc.pubKeyB64.isEmpty()) {
            emit messageFailed(m_pendingMessageId, "encryptio failed");
            return;
        }

        // update local cipher text
        if (m_messages.contains(m_pendingMessageId)) {
            Message &m = m_messages[m_pendingMessageId];
            m.cipherText = enc.cipherTextB64;
            emit messageUpdated(m);
        }

        QJsonObject body;
        body["recipient_user_id"] = m_pendingPeerUserId;
        body["recipient_device_id"] = m_pendingPeerDeviceId;
        body["cipher_text"] = enc.cipherTextB64;
        body["pub_key"] = enc.pubKeyB64;
        if (enc.x3dhOtpId.has_value()) {
            body["x3dh_otpk_id"] = enc.x3dhOtpId.value();
        }

        qDebug() << "[MessageService] POST /messages/send" << body;

        m_httpClient->post("/messages/send", body);
    };

    // if session is ready - encrypt/send
    if (!peerDeviceId.isEmpty()) {
        if (CryptoService::instance().ensureSession(peerUserId, peerDeviceId, QJsonObject())) {
            encryptAndSend();
            return;
        }
    }

    // otherwise fetch bundle and create session
    KeyService::instance().fetchBundle(peerUserId);

    m_pendingAfterBundle = encryptAndSend;
}

void MessageService::markDelivered(const QString &msgId) {
    if (!m_messages.contains(msgId)) {
        return;
    }

    QJsonObject body;
    body["delivered"] = QJsonArray{ msgId };

    m_wsClient->send("ack_delivered", body);
}

void MessageService::markRead(const QString &msgId) {
    if (!m_messages.contains(msgId)) {
        return;
    }

    QJsonObject body;
    body["read"] = QJsonArray{ msgId };

    m_wsClient->send("ack_read", body);
}

void MessageService::loadHistory(const QString &peerUserId, const int limit) {
    if (peerUserId.isEmpty()) {
        return;
    }

    m_historyPeerId = peerUserId;

    // /messages/history/{peer_id}?limit=50
    const QString path = QString("/messages/history/%1?limit=%2").arg(peerUserId).arg(limit);
    qDebug() << "[MessageService] load history GET " << path;

    m_historyHttpClient->get(path);
}

void MessageService::markAllReadForPeer(const QString &peerUserId) {
    QJsonArray ids;

    for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
        Message &msg = it.value();
        if (!msg.isOutgoing && msg.peerUserId == peerUserId && msg.status != MessageStatus::Read) {
            ids.append(msg.id);
            msg.status = MessageStatus::Read;

            emit messageUpdated(msg);
        }
    }

    if (!ids.isEmpty()) {
        QJsonObject body;
        body["read"] = ids;
        m_wsClient->send("ack_read", body);
    }
}

void MessageService::handleIncoming(const QString &type, const QJsonObject &json) {
    if (type == "message") {
        Message msg;
        msg.id = json["id"].toString();
        msg.peerUserId = json["sender_user_id"].toString();
        msg.cipherText = json["cipher_text"].toString();
        msg.isOutgoing = false;
        msg.status = MessageStatus::Delivered;
        msg.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);

        const QString uid = AuthSession::instance().userId();
        if (!CryptoService::instance().ensureSession(msg.peerUserId, json["sender_device_id"].toString(), nullptr)) {
            emit messageFailed(msg.id, "Session failed on handle incoming.");
            return;
        }
        msg.plainText = CryptoService::instance().decryptFromDevice(
            msg.peerUserId,
            json["sender_device_id"].toString(),
            json["pub_key"].toString(),
            msg.cipherText
            );

        m_messages[msg.id] = msg;

        emit messageAdded(msg);

        markDelivered(msg.id);

        return;
    }

    if (type == "message:delivered") {
        if (const QString id = json["message_id"].toString(); m_messages.contains(id)) {
            Message &msg = m_messages[id];
            msg.status = MessageStatus::Delivered;

            emit messageUpdated(msg);
            return;
        }
    }

    if (type == "message:read") {
        const QString id = json["message_id"].toString();
        if (!m_messages.contains(id)) {
            return;
        }

        Message &msg = m_messages[id];
        msg.status = MessageStatus::Read;

        emit messageUpdated(msg);
        return;
    }
}

void MessageService::onHttpSuccess(const QJsonDocument &doc) {
    if (!doc.isObject()) {
        return;
    }

    const auto obj = doc.object();
    const QString msgId = obj["message_id"].toString();

    if (!m_messages.contains(msgId)) {
        return;
    }

    Message &msg = m_messages[msgId];
    msg.status = MessageStatus::Sent;

    emit messageUpdated(msg);
}

void MessageService::onHttpError(const QString &err) {
    qWarning() << "[MessageService] send failed: " << err;
}

void MessageService::onHistoryHttpSuccess(const QJsonDocument &doc) {
    QVector<Message> out;
    if (!doc.isArray()) {
        emit historyFailed(m_historyPeerId, "History response is not an array.");
        return;
    }

    const QString currentUid = AuthSession::instance().userId();
    const auto arr = doc.array();

    out.reserve(arr.size());

    for (const auto &v : arr) {
        if (!v.isObject()) continue;
        const auto o = v.toObject();

        Message msg;
        msg.id = o.value("id").toString();
        msg.cipherText = o.value("cipher_text").toString();
        msg.createdAt = QDateTime::fromString(o.value("created_at").toString(), Qt::ISODate);

        const QString senderId = o.value("sender_user_id").toString();
        if (senderId == currentUid) {
            // outgoing
            msg.isOutgoing = true;
            msg.peerUserId = m_historyPeerId;
        } else {
            msg.isOutgoing = false;
            msg.peerUserId = senderId;
        }

        msg.isOutgoing = (senderId == currentUid);

        // status: no delivered_at / read_at
        const bool hasRead = !o.value("read_at").toString().isEmpty();
        const bool hasDelivered = !o.value("delivered_at").toString().isEmpty();

        if (msg.isOutgoing) {
            if (hasRead) msg.status = MessageStatus::Read;
            else if (hasDelivered) msg.status = MessageStatus::Delivered;
            else msg.status = MessageStatus::Sent;
        } else {
            msg.status = MessageStatus::Delivered;
        }

        // decrypt
        if (!msg.cipherText.isEmpty()) {
            const QString senderDeviceId = o.value("sender_Device_id").toString();
            if (!CryptoService::instance().ensureSession(msg.peerUserId, senderDeviceId, QJsonObject())) {
                emit messageFailed(msg.id, "Session not active on handling history.");
                return;
            }
            msg.plainText = CryptoService::instance().decryptFromDevice(
                msg.peerUserId,
                senderDeviceId,
                o.value("pub_key").toString(),
                msg.cipherText
                );
        }

        m_messages[msg.id] = msg;

        out.push_back(msg);
    }

    std::sort(out.begin(), out.end(), [](const Message &a, const Message &b) {
        return a.createdAt < b.createdAt;
    });

    emit historyLoaded(m_historyPeerId, out);
}

void MessageService::onHistoryHttpError(const QString &err) {
}
