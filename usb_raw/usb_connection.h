#pragma once

#include "usbface.h"
#include "hid_device.h"
#include <QtCore/QObject>

class QTimer;
class QMutex;

class ConnectionObject : public QObject
{
    Q_OBJECT

public:
    ConnectionObject();

    virtual bool isConnected() const = 0;

    virtual void connectToDevice() = 0;
    virtual void disconnectFromDevice() = 0;
};

class UsbConnection : public ConnectionObject
{
    Q_OBJECT

public:
    UsbConnection();
    virtual ~UsbConnection();

    bool isConnected() const override;

    void connectToDevice() override;
    void disconnectFromDevice() override;

    hid_device *getDevHandle();

signals:
    void signalConnectionChanged(bool connected);
    void signalConnected();

protected:
    bool openHidDeviceByPath(std::string devicePath);
    bool checkProtocolVersion(hid_device *device);

private slots:
    void onTimeout();

private:
    hid_device *_hid_device = 0;

    QTimer *_timer = 0;
    QMutex *_mutex = 0;
};
