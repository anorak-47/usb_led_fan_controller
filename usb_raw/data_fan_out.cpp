#include "data_fan_out.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include "usbface.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>

DataFanOut::DataFanOut(int channel) : DataWithAChannel(channel)
{
    memset(&_fanout, 0, sizeof(Fan_out));
}

DataFanOut::~DataFanOut()
{
}

QString DataFanOut::name() const
{
    return QString("Fan Output %1").arg(_channel + 1);
}

QString DataFanOut::description() const
{
    return usbfaceFanOutModeToString(static_cast<FANOUTMODE>(_fanout.mode));
}

void DataFanOut::update()
{
    if (_enabled)
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateFanOut>(new CommandUpdateFanOut(this))));
}

void DataFanOut::updateValues()
{
    if (_enabled)
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandUpdateFanOutValue>(new CommandUpdateFanOutValue(this))));
}

void DataFanOut::updateMode(FANOUTMODE mode)
{
    QMutexLocker l(_mutex);
    _fanout.mode = mode;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanOutMode>(new CommandSetFanOutMode(this))));
}

Fan_out &DataFanOut::data()
{
    return _fanout;
}

bool DataFanOut::handleEvent(CommandEvent *event)
{
    if (event->type() == (QEvent::Type)CommandEvents::EventValueUpdated)
    {
        //qDebug() << "EventValueUpdated event, channel: " << event->getChannel();

        if (event->getChannel() == _channel)
        {
            //qDebug() << "EventValueUpdated: " << fullName();
            emit signalValueChanged();
            return true;
        }
    }

    return false;
}
