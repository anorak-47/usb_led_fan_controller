#include "data_powermeter.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>

DataPowerMeter::DataPowerMeter(int channel) : DataWithAChannel(channel), _power_mW(0), _current_mA(0), _load_mV(0)
{
    _seriesCurrent.setCapacity(TIME_SERIES_CAPACITY);
    _seriesLoad.setCapacity(TIME_SERIES_CAPACITY);
    _seriesPower.setCapacity(TIME_SERIES_CAPACITY);
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
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdatePowerMeter>(new CommandUpdatePowerMeter(this))));
}

void DataPowerMeter::updateValues()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandUpdatePowerMeter>(new CommandUpdatePowerMeter(this))));
}

void DataPowerMeter::setCurrent_mA(unsigned int current)
{
    QMutexLocker l(_mutex);
    _current_mA = current;
    _seriesCurrent.append(TimeSeriesData(current));
}

void DataPowerMeter::setLoad_mV(unsigned int load)
{
    QMutexLocker l(_mutex);
    _load_mV = load;
    _seriesLoad.append(TimeSeriesData(load));
}

void DataPowerMeter::setPower_mW(unsigned int power)
{
    QMutexLocker l(_mutex);
    _power_mW = power;
    _seriesPower.append(TimeSeriesData(power));
}

QContiguousCache<TimeSeriesData> DataPowerMeter::seriesPower()
{
    return _seriesPower;
}

QContiguousCache<TimeSeriesData> DataPowerMeter::seriesCurrent()
{
    return _seriesCurrent;
}

QContiguousCache<TimeSeriesData> DataPowerMeter::seriesLoad()
{
    return _seriesLoad;
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
        qDebug() << " EventValueUpdated event";

        emit signalValueChanged();
        return true;
    }

    return false;
}
