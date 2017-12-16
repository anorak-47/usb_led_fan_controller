#include "usb_connection.h"
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QDebug>

ConnectionObject::ConnectionObject()
{
}

UsbConnection::UsbConnection()
{
    _mutex = new QMutex();
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

UsbConnection::~UsbConnection()
{
    delete _timer;
    delete _mutex;
}

bool UsbConnection::isConnected() const
{
    QMutexLocker l(_mutex);
	return (m_device != NULL);
}

void UsbConnection::connectToDevice()
{
    onTimeout();
    _timer->start(1000);
}

void UsbConnection::disconnectFromDevice()
{
    if (m_device != NULL)
    {
        if (usbfaceClose(m_device) == USBFACE_SUCCESS)
        {
            m_device = NULL;
        }
    }
}

usb_dev_handle *UsbConnection::getDevHandle()
{
    QMutexLocker l(_mutex);
	return m_device;
}

void UsbConnection::onTimeout()
{    
    bool isOpen = m_device != NULL;
    bool wasClosed = !isOpen;
    // If USB interface is not open, try to open it (m_device will be set)
    if (!isOpen)
    {
        QMutexLocker l(_mutex);
        isOpen = usbfaceOpen(&m_device) == USBFACE_SUCCESS;
    }
    // If USB interface is open, ping it to test correct operation.
    // If ping fails isOpen will be set false.
    if (isOpen && m_device)
        isOpen = usbfacePing(m_device) == USBFACE_SUCCESS;
    // If USB interface is not open and m_device is set,
    // try to close it.
    if (!isOpen && m_device != NULL)
    {
        if (usbfaceClose(m_device) == USBFACE_SUCCESS)
        {
            QMutexLocker l(_mutex);
            m_device = NULL;
        }
    }

    // Enable/disable tabs, only act on changes of the USB connection
    static bool starting = true;
    if (isOpen == wasClosed || starting)
    {
        qDebug() << "UsbConnection: is connected: " << isOpen;
        starting = false;
        emit signalConnectionChanged(isOpen);
        if (isOpen)
            emit signalConnected();
    }

#if 0
    // Update statusbar
    m_statusBar->SetStatusText(isOpen ? wxT("Connected") : wxT("Disconnected"), 1 );

    SUPPORTED funcs = SUPPORTED_NONE;
    if (isOpen)
    {
        usbfaceFuncsSupportedRead(m_device, &funcs);
    }
    // Enable/disable tabs, only act on changes of the USB connection
    static bool starting = true;
    if (isOpen == wasClosed || starting)
    {
        starting = false;
        for (unsigned int i = 0; i < m_ntbMain->GetPageCount(); i++)
        {
            wxNotebookPage* p = m_ntbMain->GetPage(i);
            if (p && p != m_pnlFanOut && i > 0 /* About is always enabled */)
            {
                p->Enable(isOpen);
            }
        }
        // Only enable fanout page when firmware support fanout.
        m_pnlFanOut->Enable( isOpen && (funcs & SUPPORTED_FAN_OUT));
    }
#endif
}
