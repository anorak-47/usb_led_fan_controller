#include "data.h"
#include "command_event.h"
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

DataObject::DataObject()
    : _mutex(new QMutex(QMutex::RecursionMode::Recursive))
{
}

DataObject::~DataObject()
{
    delete _mutex;
}

QString DataObject::fullName() const
{
    return QString("%1 - %2").arg(name()).arg(description());
}

QMutex *DataObject::mutex() const
{
    return _mutex;
}

bool DataObject::handleEvent(CommandEvent *event)
{
    Q_UNUSED(event);
    return false;
}

void DataObject::on_update()
{
    update();
}

bool DataObject::event(QEvent *event)
{
    //qDebug() << "DataObject::event " << event->type();

    if (event->type() >= (QEvent::Type)CommandEvents::EventAllDataUpdated)
    {
        CommandEvent *myEvent = static_cast<CommandEvent *>(event);

        if ((event->type() == (QEvent::Type)CommandEvents::EventAllDataUpdated))
        {
            //qDebug() << " EventAllDataUpdated event, channel: " << myEvent->getChannel();
            emit signalChanged();
            return true;
        }

        if (event->type() == (QEvent::Type)CommandEvents::EventUsbCommunicationError)
        {
            CommandEventUsbError *usbErrorEvent = static_cast<CommandEventUsbError *>(event);
            //qDebug() << " EventUsbCommunicationError event, rc: " << usbErrorEvent->errorCode() << " " << usbErrorEvent->errorMsg();
            emit signalUsbCommunicationFailed(usbErrorEvent->errorCode(), usbErrorEvent->errorMsg());
            return true;
        }

		if (handleEvent(myEvent))
			return true;
    }

    return QObject::event(event);
}

DataWithAChannel::DataWithAChannel(int channel)
    : _channel(channel)
{
}

DataWithAChannel::~DataWithAChannel()
{
}

int DataWithAChannel::channel() const
{
    return _channel;
}

bool DataWithAChannel::event(QEvent *event)
{
    //qDebug() << "DataWithAChannel::event " << event->type();

    if (event->type() >= (QEvent::Type)CommandEvents::EventAllDataUpdated)
    {
        CommandEvent *myEvent = static_cast<CommandEvent *>(event);

        if (myEvent->getChannel() == _channel)
        {
            return DataObject::event(event);
        }
    }

    return QObject::event(event);
}
