#include "data_powermeter.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>

DataPowerMeter::DataPowerMeter(int channel) : DataWithAChannel(channel), _power_mW(0), _current_mA(0), _load_mV(0)
{
}

DataPowerMeter::~DataPowerMeter()
{
}

QString DataPowerMeter::name() const
{
    return QString("Power Meter %1").arg(_channel + 1);
}

QString DataPowerMeter::description() const
{
    return "INA260";
}

void DataPowerMeter::update()
{
    if (_enabled)
        CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdatePowerMeter>(new CommandUpdatePowerMeter(this))));
}

void DataPowerMeter::updateValues()
{
    if (_enabled)
        CommandQueueInstance().enqueue(
                    std::move(std::unique_ptr<CommandUpdatePowerMeter>(new CommandUpdatePowerMeter(this))));
}

void DataPowerMeter::setCurrent_mA(unsigned int current)
{
    QMutexLocker l(_mutex);
    _current_mA = current;
}

void DataPowerMeter::setLoad_mV(unsigned int load)
{
    QMutexLocker l(_mutex);
    _load_mV = load;
}

void DataPowerMeter::setPower_mW(unsigned int power)
{
    QMutexLocker l(_mutex);
    _power_mW = power;
}

unsigned int DataPowerMeter::getCurrent_mA() const
{
    QMutexLocker l(_mutex);
    return _current_mA;
}

unsigned int DataPowerMeter::getLoad_mV() const
{
    QMutexLocker l(_mutex);
    return _load_mV;
}

unsigned int DataPowerMeter::getPower_mW() const
{
    QMutexLocker l(_mutex);
    return _power_mW;
}

bool DataPowerMeter::handleEvent(CommandEvent *event)
{
    if (event->type() == (QEvent::Type)CommandEvents::EventValueUpdated)
    {
        qDebug() << "EventValueUpdated event, channel: " << event->getChannel();

        if (event->getChannel() == _channel)
        {
            qDebug() << "EventValueUpdated: " << fullName();
            emit signalValueChanged();
            return true;
        }
    }

    return false;
}
