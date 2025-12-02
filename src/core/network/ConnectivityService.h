#ifndef CORE_NETWORK_CONNECTIVITY_SERVICE_H
#define CORE_NETWORK_CONNECTIVITY_SERVICE_H

#include <QObject>
#include <QTimer>

#include "HttpClient.h"

class ConnectivityService final : public QObject {
    Q_OBJECT
public:
    enum State {
        Unknown,
        Connecting,
        Online,
        Offline,
    };

    explicit ConnectivityService(QObject *parent = nullptr);
    void start();
    State state() const { return m_state; };
    bool isOnline() const { return m_state == Online; };

signals:
    void serverOnline();
    void serverOffline();
    void stateChanged(State s);

private:
    HttpClient *m_httpClient;
    QTimer m_retryTimer;
    State m_state = Unknown;

    void check();
    void setState(State s);
};

#endif //CORE_NETWORK_CONNECTIVITY_SERVICE_H
