#include "data_fan.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include "usbface.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>
#include <math.h>

DataFan::DataFan(int channel) : DataWithAChannel(channel)
{
    memset(&_fan, 0, sizeof(Fan));

    _seriesRpm.setCapacity(TIME_SERIES_CAPACITY);
    _seriesDuty.setCapacity(TIME_SERIES_CAPACITY);
    _seriesSetpoint.setCapacity(TIME_SERIES_CAPACITY);
}

DataFan::~DataFan()
{
}

QString DataFan::name() const
{
    return QString("Fan %1").arg(_channel + 1);
}

QString DataFan::description() const
{
    return QString("%1 (%2) - %3")
        .arg(usbfaceFanTypeToString(static_cast<FANTYPE>(_fan.config.fanType), 0))
        .arg(usbfaceFanTypeToString(static_cast<FANTYPE>(_fan.config.fanType), 1))
        .arg(usbfaceFanModeToString(static_cast<FANMODE>(_fan.config.fanMode)));
}

void DataFan::update()
{
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateFan>(new CommandUpdateFan(this))));
}

void DataFan::updateValues()
{
    if (_fan.config.fanType != FANTYPE_NONE && _fan.config.fanType != FANTYPE_2WIRE)
        CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateFanValue>(new CommandUpdateFanValue(this))));
}

void DataFan::setScaledDuty(double duty)
{
    QMutexLocker l(_mutex);
    _seriesSetpoint.append(TimeSeriesData(duty));
    _scaledDuty = duty;
}

double DataFan::getScaledDuty() const
{
    QMutexLocker l(_mutex);
    return _scaledDuty;
}

void DataFan::setRPM(unsigned int rpm)
{
    QMutexLocker l(_mutex);
    _fan.rpm = rpm;
    _fan.rps = round(rpm / 60.0);
    _seriesRpm.append(TimeSeriesData(rpm));
}

unsigned int DataFan::getRPM() const
{
    QMutexLocker l(_mutex);
    return _fan.rpm;
}

void DataFan::setScaledFixedDuty(double duty)
{
    QMutexLocker l(_mutex);
    _scaledFixedDuty = duty;
}

double DataFan::getScaledFixedDuty() const
{
    QMutexLocker l(_mutex);
    return _scaledFixedDuty;
}

void DataFan::setMinRpmStalled(double rpm)
{
    QMutexLocker l(_mutex);
    _minRpmStalled = rpm;
}

double DataFan::getMinRpmStalled() const
{
    QMutexLocker l(_mutex);
    return _minRpmStalled;
}

double DataFan::getSetpointValue() const
{
    return _setpointValue;
}

void DataFan::setSetpointValue(double setpointValue)
{
    _seriesSetpoint.append(TimeSeriesData(setpointValue));
    _setpointValue = setpointValue;
}

double DataFan::getPiSetpointOffset() const
{
    return _piSetpointOffset;
}

void DataFan::setPiSetpointOffset(double piSetpointOffset)
{
    _piSetpointOffset = piSetpointOffset;
}

double DataFan::getPiControllerKt() const
{
    return _piControllerKt;
}

double DataFan::getPiControllerKi() const
{
    return _piControllerKi;
}

double DataFan::getPiControllerKp() const
{
    return _piControllerKp;
}

double DataFan::getLinearOffset() const
{
    return _linearOffset;
}

void DataFan::setLinearOffset(double linearOffset)
{
    _linearOffset = linearOffset;
}

void DataFan::setPiControllerParameters(double kp, double ki, double kt)
{
    _piControllerKi = ki;
    _piControllerKp = kp;
    _piControllerKt = kt;
}

double DataFan::getLinearGain() const
{
    return _linearGain;
}

void DataFan::setLinearGain(double linearGain)
{
    _linearGain = linearGain;
}

void DataFan::updateType(FANTYPE type)
{
    QMutexLocker l(_mutex);
    _fan.config.fanType = type;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateMode(FANMODE mode)
{
    QMutexLocker l(_mutex);
    _fan.config.fanMode = mode;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateSensorIndex(int index)
{
    QMutexLocker l(_mutex);
    _fan.config.snsIdx = index;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateMinRpmStalled(double rpm)
{
    setMinRpmStalled(rpm);

    QMutexLocker l(_mutex);
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateMinDuty(int duty)
{
    QMutexLocker l(_mutex);
    _fan.dutyMin = duty;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateMaxDuty(int duty)
{
    QMutexLocker l(_mutex);
    _fan.dutyMax = duty;
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandSetFanConfig>(new CommandSetFanConfig(this))));
}

void DataFan::updateFixedDuty(int duty)
{
    QMutexLocker l(_mutex);
    _fan.dutyFixed = round(duty / 100.0 * 255.0);

    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanFixedDutyController>(new CommandSetFanFixedDutyController(this))));
}

void DataFan::updatePiControllerParameters(double kp, double ki, double kt)
{
    QMutexLocker l(_mutex);
    _fan.Kp = kp;
    _fan.Ki = ki;
    _fan.Kt = kt;

    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanPiController>(new CommandSetFanPiController(this))));
}

void DataFan::updatePiControllerSetpoint(int snsIdx, double offset)
{
    QMutexLocker l(_mutex);
    _fan.snsSetp.delta = round(offset);
    _fan.snsSetp.snsIdx = snsIdx;

    _piSetpointOffset = offset;

    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanPiController>(new CommandSetFanPiController(this))));
}


double DataFan::getPiControllerDebugI() const
{
    return _piControllerDebugI;
}

double DataFan::getPiControllerDebugP() const
{
    return _piControllerDebugP;
}

double DataFan::getPiControllerDebugE() const
{
    return _piControllerDebugE;
}

void DataFan::updateLinearGain(double gain)
{
    QMutexLocker l(_mutex);
    _linearGain = gain;

    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanLinearController>(new CommandSetFanLinearController(this))));
}

void DataFan::updateLinearOffset(double offset)
{
    QMutexLocker l(_mutex);
    _linearOffset = offset;

    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanLinearController>(new CommandSetFanLinearController(this))));
}

void DataFan::updateTripPoints()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFanTripPointController>(new CommandSetFanTripPointController(this))));
}

Fan &DataFan::data()
{
    return _fan;
}

QContiguousCache<TimeSeriesData> DataFan::seriesRpm() const
{
    return _seriesRpm;
}

QContiguousCache<TimeSeriesData> DataFan::seriesDuty() const
{
    return _seriesDuty;
}

QContiguousCache<TimeSeriesData> DataFan::seriesSetpoint() const
{
    return _seriesSetpoint;
}

bool DataFan::handleEvent(CommandEvent *event)
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
