#include "MessageService.h"

#include <QJsonArray>

#include "core/auth/AuthSession.h"
#include "core/crypto/CryptoService.h"

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
}

void MessageService::sendMessage(const QString &peerUserId, const QString &text) {
    Message msg;
    msg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    msg.peerUserId = peerUserId;
    msg.plainText = text;
    msg.status = MessageStatus::Sending;
    msg.isOutgoing = true;
    msg.createdAt = QDateTime::currentDateTimeUtc();

    const QString uid = AuthSession::instance().userId();
    msg.cipherText = CryptoService::instance().encryptForChat(uid, peerUserId, text);

    m_messages[msg.id] = msg;

    emit messageAdded(msg); // show in UI

    QJsonObject body;
    body["recipient_user_id"] = peerUserId;
    body["cipher_text"] = msg.cipherText;
    qDebug() << "[MessageService] POST /messages/send" << body;
    m_httpClient->post("/messages/send", body);
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
        msg.plainText = CryptoService::instance().decryptForChat(uid, msg.peerUserId, msg.cipherText);

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
            msg.plainText = CryptoService::instance().decryptForChat(currentUid, msg.peerUserId, msg.cipherText);
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
