#include "usb_connection.h"
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QDebug>

ConnectionObject::ConnectionObject()
{
    hid_init();
}

UsbConnection::UsbConnection()
{
    _mutex = new QMutex(QMutex::RecursionMode::Recursive);
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

UsbConnection::~UsbConnection()
{
    disconnectFromDevice();
    hid_exit();
    delete _timer;
    delete _mutex;
}

bool UsbConnection::isConnected() const
{
    QMutexLocker l(_mutex);
    return (_hid_device != 0);
}

bool UsbConnection::checkProtocolVersion(hid_device *device)
{
    bool protocol_version_is_valid = false;
    unsigned char version;
    int rc = usbfaceProtocolVersion(device, &version);
    if (rc == USBFACE_SUCCESS)
    {
        qDebug() << "protocol version: " << version;
        protocol_version_is_valid = (version == USB_PROTOCOL_VERSION);
    }

    if (!protocol_version_is_valid)
    {
        qDebug() << "protocol version valid: " << protocol_version_is_valid;
        qDebug() << "update the firmware of the device!";
    }

    return protocol_version_is_valid;
}

bool UsbConnection::openHidDeviceByPath(std::string devicePath)
{
    _hid_device = hid_open_device_by_path(devicePath);

    if (_hid_device && !checkProtocolVersion(_hid_device))
    {
        disconnectFromDevice();
    }

    return (_hid_device != 0);
}

void UsbConnection::connectToDevice()
{
#if 0
    std::vector<std::string> devicePaths = hid_get_device_paths(DEVICE_VID, DEVICE_PID, DEVICE_INTERFACE_NUMBER);

    if (devicePaths.size() == 1)
    {
        onTimeout();
        _timer->start(1000);
    }
#else
    onTimeout();
    _timer->start(1000);
#endif
}

void UsbConnection::disconnectFromDevice()
{
    QMutexLocker l(_mutex);
    if (_hid_device != NULL)
    {
        qDebug() << "close device";
        hid_close(_hid_device);
        _hid_device = 0;
    }
}

hid_device *UsbConnection::getDevHandle()
{
    QMutexLocker l(_mutex);
    return _hid_device;
}

void UsbConnection::onTimeout()
{    
    bool isOpen = _hid_device != NULL;
    bool wasClosed = !isOpen;

    // If USB interface is not open, try to open it (_hid_device will be set)
    if (!isOpen)
    {
        std::vector<std::string> devicePaths = hid_get_device_paths(DEVICE_VID, DEVICE_PID, DEVICE_INTERFACE_NUMBER);

        if (devicePaths.size() == 1)
        {
            isOpen = openHidDeviceByPath(devicePaths.front());
        }
        else
        {
            //TODO: show a selection dialog
        }
    }

    // If USB interface is open, ping it to test correct operation.
    // If ping fails isOpen will be set false.
    if (isOpen && _hid_device)
    {
        //isOpen = usbfacePing(_hid_device) == USBFACE_SUCCESS;
    }

    // If USB interface is not open and m_device is set,
    // try to close it.
    if (!isOpen && _hid_device != NULL)
    {
        disconnectFromDevice();
    }

    // Enable/disable tabs, only act on changes of the USB connection
    static bool starting = true;
    if (isOpen == wasClosed || starting)
    {
        qDebug() << "UsbConnection: -------------> is connected: " << isOpen;
        starting = false;
        emit signalConnectionChanged(isOpen);
        if (isOpen)
            emit signalConnected();       
    }
}
