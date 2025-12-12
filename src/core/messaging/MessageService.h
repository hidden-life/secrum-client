#ifndef CORE_MESSAGING_MESSAGE_SERVICE_H
#define CORE_MESSAGING_MESSAGE_SERVICE_H

#include <QObject>

#include "core/model/Message.h"
#include "core/network/HttpClient.h"

class MessageService : public QObject {
    Q_OBJECT
public:
    explicit MessageService(QObject *parent = nullptr);
    void sendMessage(const QString &peerUserId, const QString &text);

signals:
    void messageAdded(const Message &message);
    void messageUpdated(const Message &message);
    void messageFailed(const QString &tmpId, const QString &reason);

private:
    HttpClient *m_httpClient;
};

#endif //CORE_MESSAGING_MESSAGE_SERVICE_H
