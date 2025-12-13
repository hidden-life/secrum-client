#ifndef CORE_NETWORK_WSCLIENT_H
#define CORE_NETWORK_WSCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QUrl>

class WSClient : public QObject {
    Q_OBJECT
public:
    explicit WSClient(QObject *parent = nullptr);

    void connectToHost(QUrl url);
    void disconnectFromHost();

    void send(const QString &type, const QJsonObject &json = {});

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);

    void eventReceived(const QString &type, const QJsonObject &data);

private:
    QWebSocket m_ws;
};

#endif //CORE_NETWORK_WSCLIENT_H
