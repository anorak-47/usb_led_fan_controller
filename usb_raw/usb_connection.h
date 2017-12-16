#pragma once

#include "usbface.h"
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

    usb_dev_handle *getDevHandle();

signals:
    void signalConnectionChanged(bool connected);
    void signalConnected();

private slots:
    void onTimeout();

private:
    usb_dev_handle *m_device = 0;
    QTimer *_timer = 0;
    QMutex *_mutex = 0;
};
