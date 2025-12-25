#ifndef CORE_MESSAGING_MESSAGE_SERVICE_H
#define CORE_MESSAGING_MESSAGE_SERVICE_H

#include <QObject>
#include <QHash>

#include "core/model/Message.h"
#include "core/network/HttpClient.h"
#include "core/network/WSClient.h"

class MessageService final : public QObject {
    Q_OBJECT
public:
    explicit MessageService(WSClient *wsClient, QObject *parent = nullptr);
    void sendMessage(const QString &peerUserId, const QString &peerDeviceId, const QString &text);
    void markDelivered(const QString &msgId);
    void markRead(const QString &msgId);
    void loadHistory(const QString &peerUserId, int limit = 50);
    void markAllReadForPeer(const QString &peerUserId);

signals:
    void messageAdded(const Message &message);
    void messageUpdated(const Message &message);
    void messageFailed(const QString &tmpId, const QString &reason);

    void historyLoaded(const QString &peerUserId, const QVector<Message> &messages);
    void historyFailed(const QString &peerUserId, const QString &reason);

private:
    WSClient *m_wsClient;
    QHash<QString, Message> m_messages;
    HttpClient *m_httpClient;
    HttpClient *m_historyHttpClient;
    QString m_historyPeerId;

    QString m_pendingPeerUserId;
    QString m_pendingPeerDeviceId;
    QString m_pendingPlainText;
    QString m_pendingMessageId;
    std::function<void()> m_pendingAfterBundle = nullptr;

    void handleIncoming(const QString &type, const QJsonObject &json);

private slots:
    void onHttpSuccess(const QJsonDocument &doc);
    void onHttpError(const QString &err);

    void onHistoryHttpSuccess(const QJsonDocument &doc);
    void onHistoryHttpError(const QString &err);
};

#endif //CORE_MESSAGING_MESSAGE_SERVICE_H
