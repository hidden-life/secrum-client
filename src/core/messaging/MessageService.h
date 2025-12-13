#ifndef CORE_MESSAGING_MESSAGE_SERVICE_H
#define CORE_MESSAGING_MESSAGE_SERVICE_H

#include <QObject>
#include <QHash>

#include "core/model/Message.h"
#include "core/network/HttpClient.h"
#include "core/network/WSClient.h"

class MessageService : public QObject {
    Q_OBJECT
public:
    explicit MessageService(WSClient *wsClient, QObject *parent = nullptr);
    void sendMessage(const QString &peerUserId, const QString &text);
    void markDelivered(const QString &msgId);
    void markRead(const QString &msgId);

signals:
    void messageAdded(const Message &message);
    void messageUpdated(const Message &message);
    void messageFailed(const QString &tmpId, const QString &reason);

private:
    WSClient *m_wsClient;
    QHash<QString, Message> m_messages;
    HttpClient *m_httpClient;

    void handleIncoming(const QString &type, const QJsonObject &json);
};

#endif //CORE_MESSAGING_MESSAGE_SERVICE_H
