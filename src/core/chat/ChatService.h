#ifndef CORE_CHAT_CHAT_SERVICE_H
#define CORE_CHAT_CHAT_SERVICE_H

#include <QObject>
#include <QVector>
#include "core/model/Chat.h"
#include "core/network/HttpClient.h"

class ChatService final : public QObject {
    Q_OBJECT
public:
    explicit ChatService(QObject *parent = nullptr);
    void fetchChats();

signals:
    void chatsLoaded(const QVector<Chat> &chats);
    void requestFailed(const QString &err);

private:
    HttpClient *m_httpClient = nullptr;
    QVector<Chat> parse(const QJsonArray &arr);
};

#endif //CORE_CHAT_CHAT_SERVICE_H
