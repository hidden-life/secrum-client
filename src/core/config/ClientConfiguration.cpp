#include "ClientConfiguration.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

ClientConfiguration &ClientConfiguration::instance() {
    static ClientConfiguration instance;
    return instance;
}

QString ClientConfiguration::baseURL() const {
    return m_baseURL;
}

void ClientConfiguration::setBaseURL(const QString &url) {
    m_baseURL = url;
}

QString ClientConfiguration::environment() const {
    return m_environment;
}

void ClientConfiguration::setEnvironment(const QString &env) {
    m_environment = env;
}

ClientConfiguration::ClientConfiguration() {
}

void ClientConfiguration::load() {
    loadDefaults(); // first we need to load default configuration

    const QString path = QCoreApplication::applicationDirPath() + "/config/config.json";
    qDebug() << QCoreApplication::applicationDirPath();
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "[CONFIG] client.json not found, using default configuration.";
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[CONFIG] failed to open config file, using defaults.";
        return;
    }

    QJsonParseError err{};
    auto doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "[CONFIG] parsing configuration JSON error: " << err.errorString();
        return;
    }

    auto root = doc.object();
    auto api = root["api"].toObject();

    if (api.contains("base_url")) {
        m_baseURL = api["base_url"].toString();
    }

    qDebug() << "[CONFIG] API base URL = " << m_baseURL;
}

void ClientConfiguration::save() {
}

QUrl ClientConfiguration::wsUrl() const {
    const QUrl url(m_baseURL);
    const QString scheme = url.scheme().toLower() == "https" ? "wss" : "ws";
    QUrl ws;
    ws.setScheme(scheme);
    ws.setHost(url.host());
    ws.setPath("/ws");

    return ws;
}

void ClientConfiguration::loadDefaults() {
    m_baseURL = "http://localhost:8080";
}
