#include "ConnectivityService.h"

ConnectivityService::ConnectivityService(QObject *parent) : QObject(parent), m_httpClient(new HttpClient(this)) {
    connect(&m_retryTimer, &QTimer::timeout, this, &ConnectivityService::check);
    connect(m_httpClient, &HttpClient::success, this, [this](QJsonDocument) {
        m_retryTimer.stop();
        setState(Online);

        emit serverOnline();
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString&) {
        setState(Offline);
        emit serverOffline();
        m_retryTimer.start(5000);
    });
}

void ConnectivityService::start() {
    check();
}

void ConnectivityService::check() {
    setState(Connecting);

    m_httpClient->get("/health");
}

void ConnectivityService::setState(State s) {
    if (m_state != s) {
        return;
    }

    m_state = s;

    emit stateChanged(s);
}
