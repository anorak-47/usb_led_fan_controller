#include "data_sensor.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include "usbface.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>
#include <math.h>

DataSensor::DataSensor(int channel) : DataWithAChannel(channel)
{
    memset(&_sensor, 0, sizeof(Sensor));
    _series.setCapacity(TIME_SERIES_CAPACITY);

    double value = 0;
    QDateTime dt = QDateTime::currentDateTime();
    dt = dt.addSecs(-1000);

    for (int i = 0; i < 1000; i++)
    {
        value += random() % 2 ? +random() % 2 : -random() % 2;
        TimeSeriesData tsd;
        tsd.dt = dt;
        tsd.value = value;
        _series.append(tsd);
        dt = dt.addSecs(1);
    }
}

DataSensor::~DataSensor()
{
}

QString DataSensor::name() const
{
    return QString("Sensor %1").arg(_channel+1);
}

QString DataSensor::description() const
{
    return usbfaceSensorTypeToString(static_cast<SNSTYPE>(_sensor.type));
}

void DataSensor::update()
{
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateSensor>(new CommandUpdateSensor(this))));
}

void DataSensor::updateValues()
{
    if (_sensor.type != SNSTYPE_NONE)
        CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateSensorValue>(new CommandUpdateSensorValue(this))));
}

void DataSensor::updateType(SNSTYPE type)
{
    QMutexLocker l(_mutex);
    _sensor.type = type;
    CommandQueueInstance().enqueue(
                std::move(std::unique_ptr<CommandSetSensorType>(new CommandSetSensorType(this))));
}

void DataSensor::updateValue(unsigned int value)
{
    QMutexLocker l(_mutex);
    _sensor.value = value;
    CommandQueueInstance().enqueue(
                std::move(std::unique_ptr<CommandSetSensorValue>(new CommandSetSensorValue(this))));
}

Sensor &DataSensor::data()
{
    return _sensor;
}

void DataSensor::setValue(double value)
{
	_sensor.value = round(value);
    _series.append(TimeSeriesData(value));
}

bool DataSensor::handleEvent(CommandEvent *event)
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

QContiguousCache<TimeSeriesData> DataSensor::series() const
{
    return _series;
}
