#include "DeviceService.h"
#include "core/network/HttpClient.h"
#include "core/storage/SQLiteStorage.h"

#include <QJsonArray>
#include <QUuid>

#include "core/auth/AuthSession.h"

DeviceService::DeviceService(QObject *parent) : QObject(parent) {
    m_httpClient = new HttpClient(this);

    connect(m_httpClient, &HttpClient::success, this, [this](const QJsonDocument &doc) {
        if (doc.isArray()) {
            // response from GET /devices - array
            QVector<Device> devices = parseDevices(doc.array());
            emit devicesLoaded(devices);
        } else if (doc.isObject()) {
            // response: {"status": "ok"}
            emit deviceActionCompleted();
        } else {
            emit requestFailed(QStringLiteral("Unexpected response format from server."));
        }
    });

    connect(m_httpClient, &HttpClient::error, this, [this](const QString &err) {
        emit requestFailed(err);
    });
}

void DeviceService::setAccessToken(const QString &accessToken) {
    m_accessToken = accessToken;
    if (m_httpClient) {
        m_httpClient->setAccessToken(accessToken);
    }
}

void DeviceService::fetchDevices() {
    if (!m_httpClient) {
        return;
    }

    m_httpClient->get("/devices");
}

void DeviceService::deactivateDevice(const QString &deviceId) {
    if (!m_httpClient) {
        return;
    }

    const QString path = QStringLiteral("/devices/%1/deactivate").arg(deviceId);

    m_httpClient->post(path, QJsonObject{});
}

void DeviceService::deleteDevice(const QString &deviceId) {
    if (!m_httpClient) {
        return;
    }

    const QString path = QStringLiteral("/devices/%1").arg(deviceId);

    m_httpClient->post(path, QJsonObject{{"_method", "DELETE"}});
}

QString DeviceService::deviceId() {
    QString id = AuthSession::instance().deviceId();
    if (id.isEmpty()) {
        return id;
    }

    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    AuthSession::instance().setDeviceId(id);

    return newId;
}

QVector<Device> DeviceService::parseDevices(const QJsonArray &arr) {
    QVector<Device> devices;
    devices.reserve(arr.size());

    for (const QJsonValue &value : arr) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject obj = value.toObject();
        Device d;
        d.id = obj.value("id").toString();
        d.name = obj.value("name").toString();
        d.platform = obj.value("platform").toString();
        d.isActive = obj.value("is_active").toBool();
        d.isCurrent = obj.value("is_current").toBool();
        d.createdAt = obj.value("created_at").toString();
        d.lastSeen = obj.value("last_seen").toString();

        devices.push_back(d);
    }

    return devices;
}
