#ifndef CORE_MODEL_DEVICE_H
#define CORE_MODEL_DEVICE_H

#include <QString>

struct Device {
    QString id;
    QString name;
    QString platform;
    bool isActive = false;
    bool isCurrent = false;
    QString createdAt;
    QString lastSeen;
};

#endif //CORE_MODEL_DEVICE_H
