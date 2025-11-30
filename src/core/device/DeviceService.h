#ifndef CORE_DEVICE_DEVICE_SERVICE_H
#define CORE_DEVICE_DEVICE_SERVICE_H

#include <QObject>

#include "core/model/Device.h"

class HttpClient;

class DeviceService final : public QObject {
    Q_OBJECT
public:
    explicit DeviceService(QObject *parent = nullptr);

    void setAccessToken(const QString &accessToken);

    void fetchDevices();
    void deactivateDevice(const QString &deviceId);
    void deleteDevice(const QString &deviceId);

signals:
    void devicesLoaded(const QVector<Device> &devices);
    void requestFailed(const QString &msg);
    void deviceActionCompleted();

private:
    HttpClient *m_httpClient = nullptr;
    QString m_accessToken;

    QVector<Device> parseDevices(const QJsonArray &arr);
};

#endif //CORE_DEVICE_DEVICE_SERVICE_H
