#include "data_sensor.h"
#include "data_properties.h"
#include "data_fan.h"
#include "data_fan_out.h"
#include "data_fastled.h"
#include "data_powermeter.h"
#include "usb_connection.h"
#include "usbface.h"
#include "command.h"
#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <math.h>

Command::Command()
{
    setName(__func__);
}

Command::~Command()
{
}

void Command::setName(QString const &name)
{
    _name = name;
}

QString Command::getName() const
{
    return _name;
}

CommandNotifyACaller::CommandNotifyACaller(QObject *caller) : _caller(caller)
{
    setName(__func__);
}

void CommandNotifyACaller::postEvent(CommandEvents eventType)
{
    CommandEvent *event = new CommandEvent(eventType);
    QCoreApplication::postEvent(_caller, event);
}

void CommandNotifyACaller::postUsbCommunicationErrorEvent(int errorCode, QString const &msg)
{
    QString errmsg = QString("%1: %2 %3").arg(getName()).arg(usbfaceErrToString(USBFACE_ERR_NOTFOUND)).arg(msg);
    CommandEvent *event = new CommandEventUsbError(errorCode, errmsg);
    QCoreApplication::postEvent(_caller, event);
}

CommandNotifyACallerUsbDevice::CommandNotifyACallerUsbDevice(QObject *caller) : CommandNotifyACaller(caller)
{
    setName(__func__);
}

void CommandNotifyACallerUsbDevice::exec(std::shared_ptr<ConnectionObject> &co)
{
    if (!co->isConnected())
    {
        QString msg;
        postUsbCommunicationErrorEvent(USBFACE_ERR_NOTFOUND, msg);
        return;
    }

    _co = std::dynamic_pointer_cast<UsbConnection>(co);
    _exec();
}

CommandUpdateDeviceProperties::CommandUpdateDeviceProperties(DataDeviceProperties *caller)
    : CommandNotifyACallerUsbDevice(caller), _properties(caller)
{
    setName(__func__);
}

void CommandUpdateDeviceProperties::_exec()
{
    qDebug() << "CommandUpdateDeviceProperties " << _properties->fullName();

    bool success = true;
    hid_device *dev = _co->getDevHandle();

    SUPPORTED funcs = SUPPORTED_NONE;
    int rc = usbfaceFuncsSupportedRead(dev, &funcs);
    if (USBFACE_SUCCESS == rc)
    {
        _properties->setSupportedFunctions(funcs);
        _properties->setNrOfFans(usbfaceGetNrOfFans());
        _properties->setNrOfSensors(usbfaceGetNrOfSensors());
        _properties->setNrOfFanOuts(MAX_FAN_OUTS);


    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFuncsSupportedRead");
        success = false;
    }

    unsigned char minor;
    unsigned char major;
    rc = usbfaceFirmwareVersion(dev, &major, &minor);
    if (USBFACE_SUCCESS == rc)
    {
        _properties->setVersionFirmwareMajor(major);
        _properties->setVersionFirmwareMinor(minor);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFirmwareVersion");
        success = false;
    }

    unsigned char version;
    rc = usbfaceProtocolVersion(dev, &version);
    if (USBFACE_SUCCESS == rc)
    {
        _properties->setVersionProtocol(version);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceProtocolVersion");
        success = false;
    }

    char deviceName[21];
    rc = usbfaceDeviceNameRead(dev, deviceName);
    if (USBFACE_SUCCESS == rc)
    {
        _properties->setDeviceDevice(deviceName);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceDeviceNameRead");
        success = false;
    }

    if (success)
    {
        postEvent(CommandEvents::EventPropertiesUpdated);
    }
}

CommandSettingsSave::CommandSettingsSave(DataDeviceProperties *caller)
    : CommandNotifyACallerUsbDevice(caller), _properties(caller)
{
    setName(__func__);
}

void CommandSettingsSave::_exec()
{
    qDebug() << "CommandSettingsSave " << _properties->fullName();
    hid_device *dev = _co->getDevHandle();

    int rc = usbfaceWriteSettings(dev);
    if (USBFACE_SUCCESS == rc)
    {
        postEvent(CommandEvents::EventCommandFinished);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceWriteSettings");
    }
}

CommandSettingsLoad::CommandSettingsLoad(bool clearSettings, DataDeviceProperties *caller)
    : CommandNotifyACallerUsbDevice(caller), _properties(caller), _clearSettings(clearSettings)
{
    setName(__func__);
}

void CommandSettingsLoad::_exec()
{
    qDebug() << "CommandSettingsLoad " << _properties->fullName();
    hid_device *dev = _co->getDevHandle();
    int rc;

    if (_clearSettings)
        rc = usbfaceDefaultSettings(dev);
    else
        rc = usbfaceReadSettings(dev);

    if (USBFACE_SUCCESS == rc)
    {
        postEvent(CommandEvents::EventCommandFinished);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceReadSettings");
    }
}

CommandPropertiesDeviceName::CommandPropertiesDeviceName(DataDeviceProperties *caller)
    : CommandNotifyACallerUsbDevice(caller), _properties(caller)
{
    setName(__func__);
}

void CommandPropertiesDeviceName::_exec()
{
    qDebug() << "CommandPropertiesDeviceName " << _properties->fullName();
    hid_device *dev = _co->getDevHandle();
    int rc;

    rc = usbfaceDeviceNameWrite(dev, qPrintable(_properties->getDeviceName()));

    if (USBFACE_SUCCESS == rc)
    {
        postEvent(CommandEvents::EventCommandFinished);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceDeviceNameWirte");
    }
}


CommandManipulateAChannel::CommandManipulateAChannel(unsigned int channel) : _channel(channel)
{
    setName(__func__);
}

unsigned int CommandManipulateAChannel::getChannel() const
{
    return _channel;
}

CommandManipulateAChannelNotifyACaller::CommandManipulateAChannelNotifyACaller(QObject *caller, unsigned int channel)
    : CommandManipulateAChannel(channel), _caller(caller)
{
    setName(__func__);
}

void CommandManipulateAChannelNotifyACaller::postEvent(CommandEvents eventType)
{
    CommandEvent *event = new CommandEvent(eventType, _channel);
    QCoreApplication::postEvent(_caller, event);
}

void CommandManipulateAChannelNotifyACaller::postUsbCommunicationErrorEvent(int errorCode, QString const &msg)
{
    QString errmsg = QString("%1: %2 %3").arg(getName()).arg(usbfaceErrToString(USBFACE_ERR_NOTFOUND)).arg(msg);
    CommandEvent *event = new CommandEventUsbError(errorCode, errmsg, _channel);
    QCoreApplication::postEvent(_caller, event);
}

CommandManipulateAChannelNotifyACallerUsbDevice::CommandManipulateAChannelNotifyACallerUsbDevice(QObject *caller,
                                                                                                 unsigned int channel)
    : CommandManipulateAChannelNotifyACaller(caller, channel)
{
    setName(__func__);
}

void CommandManipulateAChannelNotifyACallerUsbDevice::exec(std::shared_ptr<ConnectionObject> &co)
{
    if (!co->isConnected())
    {
        QString msg;
        postUsbCommunicationErrorEvent(USBFACE_ERR_NOTFOUND, msg);
        return;
    }

    _co = std::dynamic_pointer_cast<UsbConnection>(co);
    _exec();
}

/*
 * ----------------- sensor ----------------------------------
 */

CommandUpdateSensor::CommandUpdateSensor(DataSensor *sensor)
    : CommandManipulateAChannelNotifyACallerUsbDevice(sensor, sensor->channel()), _sensor(sensor)
{
    setName(__func__);
}

void CommandUpdateSensor::_exec()
{
    qDebug() << "CommandUpdateSensor " << _sensor->fullName();

    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    SNSTYPE type;
    int rc = usbfaceSnsTypeRead(dev, _channel, &type);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_sensor->mutex());
        _sensor->data().type = type;
        _sensor->data().status.valid = (_sensor->data().type == SNSTYPE_NONE) ? 0 : 1;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsTypeRead");
        updated = false;
    }

    double snsvalue;
    rc = usbfaceSnsRead(dev, _channel, &snsvalue);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_sensor->mutex());
        _sensor->data().value = round(snsvalue);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsRead");
        updated = false;
    }

    if (updated)
    {
        postEvent(CommandEvents::EventAllDataUpdated);
    }
}

CommandUpdateSensorValue::CommandUpdateSensorValue(DataSensor *sensor) : CommandUpdateSensor(sensor)
{
    setName(__func__);
}

void CommandUpdateSensorValue::_exec()
{
    qDebug() << "CommandUpdateSensorValue " << _sensor->fullName();

    hid_device *dev = _co->getDevHandle();

    double snsvalue;
    int rc = usbfaceSnsRead(dev, _channel, &snsvalue);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_sensor->mutex());
        _sensor->setValue(round(snsvalue));
        postEvent(CommandEvents::EventValueUpdated);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsRead");
    }
}

CommandSetSensorType::CommandSetSensorType(DataSensor *sensor) : CommandUpdateSensor(sensor)
{
    setName(__func__);
}

void CommandSetSensorType::_exec()
{
    qDebug() << "CommandSetSensorType " << _sensor->fullName();

    hid_device *dev = _co->getDevHandle();
    SNSTYPE type;

    _sensor->mutex()->lock();
    type = (SNSTYPE)_sensor->data().type;
    _sensor->data().status.valid = (type == SNSTYPE_NONE) ? 0 : 1;
    _sensor->mutex()->unlock();

    int rc = usbfaceSnsTypeWrite(dev, _channel, type);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsTypeWrite");
    }
}

CommandSetSensorValue::CommandSetSensorValue(DataSensor *sensor) : CommandUpdateSensor(sensor)
{
    setName(__func__);
}

void CommandSetSensorValue::_exec()
{
    qDebug() << "CommandSetSensorValue " << _sensor->fullName();

    hid_device *dev = _co->getDevHandle();
    double value;

    _sensor->mutex()->lock();
    value = _sensor->data().value;
    _sensor->mutex()->unlock();

    int rc = usbfaceSnsWrite(dev, _channel, value);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsWrite");
    }
}

/*
 * ----------------- fanout ----------------------------------
 */

CommandUpdateFanOut::CommandUpdateFanOut(DataFanOut *fanout)
    : CommandManipulateAChannelNotifyACallerUsbDevice(fanout, fanout->channel()), _fanout(fanout)
{
    setName(__func__);
}

void CommandUpdateFanOut::_exec()
{
    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    FANOUTMODE mode;
    int rc = usbfaceFanOutModeRead(dev, _channel, &mode);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fanout->mutex());
        _fanout->data().mode = mode;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanOutModeRead");
        updated = false;
    }

    double rpm;
    rc = usbfaceFanOutRpmRead(dev, _channel, &rpm);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fanout->mutex());
        _fanout->data().rpm = round(rpm);
        _fanout->data().rps = round(rpm / 60.0);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanOutRpmRead");
        updated = false;
    }

    _fanout->data().fanStallDetect = 0;

    for (int i = 0; i < MAX_FANS; i++)
    {
        int stalldetect;
        rc = usbfaceFanStallDetectRead(dev, i, &stalldetect);
        if (USBFACE_SUCCESS == rc)
        {
            QMutexLocker l(_fanout->mutex());
            _fanout->data().fanStallDetect |= (1 << i);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFanStallDetectRead");
            updated = false;
        }
    }

    if (updated)
    {
        postEvent(CommandEvents::EventAllDataUpdated);
    }
}

CommandUpdateFanOutValue::CommandUpdateFanOutValue(DataFanOut *fanout) : CommandUpdateFanOut(fanout)
{
    setName(__func__);
}

void CommandUpdateFanOutValue::_exec()
{
    qDebug() << "CommandUpdateFanOutValue " << _fanout->fullName();

    hid_device *dev = _co->getDevHandle();

    double rpm;
    int rc = usbfaceFanOutRpmRead(dev, _channel, &rpm);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fanout->mutex());
        _fanout->data().rpm = round(rpm);
        _fanout->data().rps = round(rpm / 60.0);
        postEvent(CommandEvents::EventValueUpdated);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanOutRpmRead");
    }
}

CommandUpdateFanOutStallDetection::CommandUpdateFanOutStallDetection(DataFanOut *fanout) : CommandUpdateFanOut(fanout)
{
    setName(__func__);
}

void CommandUpdateFanOutStallDetection::_exec()
{
    qDebug() << "CommandUpdateFanOutStallDetection " << _fanout->fullName();
    hid_device *dev = _co->getDevHandle();

    for (int i = 0; i < MAX_FANS; i++)
    {
        int stalldetect = (_fanout->data().fanStallDetect & (1 << i)) ? 1 : 0;
        int rc = usbfaceFanStallDetectWrite(dev, i, stalldetect);
        if (USBFACE_SUCCESS != rc)
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFanStallDetectWrite");
        }
    }
}

CommandSetFanOutMode::CommandSetFanOutMode(DataFanOut *fanout) : CommandUpdateFanOut(fanout)
{
    setName(__func__);
}

void CommandSetFanOutMode::_exec()
{
    qDebug() << "CommandSetFanOutMode " << _fanout->fullName();

    hid_device *dev = _co->getDevHandle();
    FANOUTMODE mode;

    _fanout->mutex()->lock();
    mode = (FANOUTMODE)_fanout->data().mode;
    _fanout->mutex()->unlock();

    int rc = usbfaceFanOutModeWrite(dev, _channel, mode);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsModeWrite");
    }
}

/*
 * ----------------- fan -------------------------------------
 */

CommandUpdateFan::CommandUpdateFan(DataFan *fan)
    : CommandManipulateAChannelNotifyACallerUsbDevice(fan, fan->channel()), _fan(fan)
{
    setName(__func__);
}

void CommandUpdateFan::_exec()
{
    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    FANMODE mode;
    int rc = usbfaceFanModeRead(dev, _channel, &mode);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().config.fanMode = mode;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanModeRead");
        updated = false;
    }

    FANTYPE type;
    rc = usbfaceFanTypeRead(dev, _channel, &type);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().config.fanType = type;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanTypeRead");
        updated = false;
    }

    int snsIdx;
    rc = usbfaceFanSensorRead(dev, _channel, &snsIdx);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().config.snsIdx = snsIdx;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanSensorRead");
        updated = false;
    }

    unsigned int rpm;
    rc = usbfaceFanRpmRead(dev, _channel, &rpm);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setRPM(rpm);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanRpmRead");
        updated = false;
    }

    unsigned char rawduty;
    rc = usbfaceFanDutyRawRead(dev, _channel, &rawduty);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().duty = rawduty;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanDutyRawRead");
        updated = false;
    }

    double duty;
    rc = usbfaceFanDutyRead(dev, _channel, &duty);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setScaledDuty(duty * 100.0);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanDutyRead");
        updated = false;
    }

    double fixedduty;
    rc = usbfaceFanDutyFixedRead(dev, _channel, &fixedduty);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setScaledFixedDuty(fixedduty * 100.0);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanDutyFixedRead");
        updated = false;
    }

    double setpoint;
    int refsnsidx;
    rc = usbfaceFanSetpointRead(dev, _channel, &setpoint, &refsnsidx);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().snsSetp.delta = round(setpoint);
        _fan->data().snsSetp.snsIdx = (SNSTYPE)refsnsidx;
        _fan->setPiSetpointOffset(setpoint);
    }
    else if (USBFACE_ERR_UNSUPP != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanSetpointRead");
        updated = false;
    }

    double setpointValue;
    rc = usbfaceFanSetpointActualRead(dev, _channel, &setpointValue);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setSetpointValue(setpointValue);
    }
    else if (USBFACE_ERR_UNSUPP != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanSetpointActualRead");
        updated = false;
    }

    double kp;
    double ki;
    double kt;
    rc = usbfaceFanPidRead(dev, _channel, &kp, &ki, &kt);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setPiControllerParameters(kp, ki, kt);
    }
    else if (USBFACE_ERR_UNSUPP != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanPidRead");
        updated = false;
    }

    double gain;
    double offs;
    rc = usbfaceFanGainOffsRead(dev, _channel, &gain, &offs);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setLinearGain(gain);
        _fan->setLinearOffset(offs);
    }
    else if (USBFACE_ERR_UNSUPP != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanGainOffsRead");
        updated = false;
    }

    int stalled;
    rc = usbfaceFanStallRead(dev, _channel, &stalled);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().status.stalled = stalled;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanStallRead");
        updated = false;
    }

    double minRpmStalled;
    rc = usbfaceFanMinRpmRead(dev, _channel, &minRpmStalled);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setMinRpmStalled(minRpmStalled);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanMinRpmRead");
        updated = false;
    }

    for (int p = 0; p < MAX_TRIP_POINTS; p++)
    {
        unsigned int value;
        unsigned int duty;
        rc = usbfaceFanTripPointRead(dev, _channel, p, &value, &duty);
        if (USBFACE_SUCCESS == rc)
        {
            _fan->data().trip_point[p].duty = duty;
            _fan->data().trip_point[p].value = value;
        }
        else if (USBFACE_ERR_UNSUPP != rc)
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFanTripPointRead");
            updated = false;
        }
    }

    /* fan_out
    int stalldetect;
    rc = usbfaceFanStallDetectRead(hid_device *device, _channel, &stalldetect);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().status.stalled = stalled;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanStallDetectRead");
        updated = false;
    }
    */

    /*
    dutyMin, dutyMax
    */

    if (updated)
    {
        postEvent(CommandEvents::EventAllDataUpdated);
    }
}

CommandUpdateFanValue::CommandUpdateFanValue(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandUpdateFanValue::_exec()
{
    qDebug() << "CommandUpdateFanValue " << _fan->fullName();

    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    unsigned int rpm;
    int rc = usbfaceFanRpmRead(dev, _channel, &rpm);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setRPM(rpm);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanRpmRead");
        updated = false;
    }

    double duty;
    rc = usbfaceFanDutyRead(dev, _channel, &duty);
    if (USBFACE_SUCCESS == rc)
    {
        _fan->setScaledDuty(duty * 100.0);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanDutyRead");
        updated = false;
    }

    int stalled;
    rc = usbfaceFanStallRead(dev, _channel, &stalled);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fan->mutex());
        _fan->data().status.stalled = stalled;
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanStallRead");
        updated = false;
    }

    if (_fan->data().config.fanMode == FANMODE_PI)
    {
        double setpointValue;
        rc = usbfaceFanSetpointActualRead(dev, _channel, &setpointValue);
        if (USBFACE_SUCCESS == rc)
        {
            _fan->setSetpointValue(setpointValue);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFanSetpointActualRead");
            updated = false;
        }
    }

    if (updated)
    {
        postEvent(CommandEvents::EventValueUpdated);
    }
}

CommandSetFanConfig::CommandSetFanConfig(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandSetFanConfig::_exec()
{
    qDebug() << "CommandSetFanConfig " << _fan->fullName();

    hid_device *dev = _co->getDevHandle();
    FANMODE mode;
    FANTYPE type;
    int snsidx;

    _fan->mutex()->lock();
    mode = (FANMODE)_fan->data().config.fanMode;
    type = (FANTYPE)_fan->data().config.fanType;
    snsidx = _fan->data().config.snsIdx;
    _fan->mutex()->unlock();

    int rc = usbfaceFanModeWrite(dev, _channel, mode);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceSnsModeWrite");
    }

    rc = usbfaceFanTypeWrite(dev, _channel, type);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanTypeWrite");
    }

    rc = usbfaceFanSensorWrite(dev, _channel, snsidx);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanSensorWrite");
    }

    rc = usbfaceFanMinRpmWrite(dev, _channel, _fan->getMinRpmStalled());
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanMinRpmWrite");
    }

    /*
    dutyMin, dutyMax
    */
}

CommandSetFanFixedDutyController::CommandSetFanFixedDutyController(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandSetFanFixedDutyController::_exec()
{
    qDebug() << "CommandSetFanFixedDutyController " << _fan->fullName();

    hid_device *dev = _co->getDevHandle();
    double duty;

    _fan->mutex()->lock();
    duty = _fan->getScaledFixedDuty() / 100.0;
    _fan->mutex()->unlock();

    qDebug() << "CommandSetFanFixedDutyController " << duty;

    int rc = usbfaceFanDutyFixedWrite(dev, _channel, duty);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanDutyFixedWrite");
    }
}

CommandSetFanLinearController::CommandSetFanLinearController(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandSetFanLinearController::_exec()
{
    qDebug() << "CommandSetFanLinearController " << _fan->fullName();

    hid_device *dev = _co->getDevHandle();
    double gain;
    double offset;

    _fan->mutex()->lock();
    gain = _fan->getLinearGain();
    offset = _fan->getLinearOffset();
    _fan->mutex()->unlock();

    int rc = usbfaceFanGainOffsWrite(dev, _channel, gain, offset);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanGainOffsWrite");
    }
}

CommandSetFanTripPointController::CommandSetFanTripPointController(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandSetFanTripPointController::_exec()
{
    qDebug() << "CommandSetFanTripPointController " << _fan->fullName();

    hid_device *dev = _co->getDevHandle();

    for (int p = 0; p < MAX_TRIP_POINTS; p++)
    {
        unsigned int value;
        unsigned int duty;

        _fan->mutex()->lock();
        value = _fan->data().trip_point[p].value;
        duty = _fan->data().trip_point[p].duty;
        _fan->mutex()->unlock();

        int rc = usbfaceFanTripPointWrite(dev, _channel, p, value, duty);
        if (USBFACE_SUCCESS != rc)
        {
            postUsbCommunicationErrorEvent(rc, "CommandSetFanTripPointController");
        }
    }
}

CommandSetFanPiController::CommandSetFanPiController(DataFan *fan) : CommandUpdateFan(fan)
{
    setName(__func__);
}

void CommandSetFanPiController::_exec()
{
    qDebug() << "CommandSetFanPiController " << _fan->fullName();

    hid_device *dev = _co->getDevHandle();

    double kp;
    double ki;
    double kt;

    double setpoint;
    int refsnsidx;

    _fan->mutex()->lock();
    kp = _fan->getPiControllerKp();
    ki = _fan->getPiControllerKi();
    kt = _fan->getPiControllerKt();
    setpoint = _fan->data().snsSetp.delta;
    refsnsidx = _fan->data().snsSetp.snsIdx;
    _fan->mutex()->unlock();

    int rc = usbfaceFanPidWrite(dev, _channel, kp, ki, kt);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanPidWrite");
    }
    rc = usbfaceFanSetpointWrite(dev, _channel, setpoint, refsnsidx);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFanSetpointWrite");
    }
}

/*
 * ----------------- fastled ----------------------------------
 */

CommandUpdateFastLed::CommandUpdateFastLed(DataFastLed *fastled)
    : CommandManipulateAChannelNotifyACallerUsbDevice(fastled, fastled->channel()), _fastled(fastled)
{
    setName(__func__);
}

void CommandUpdateFastLed::_exec()
{
    qDebug() << "CommandUpdateFastLed " << _fastled->fullName();

    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    unsigned char id;
    int rc = usbfaceFastledAnimationIdRead(dev, _channel, &id);
    if (USBFACE_SUCCESS == rc)
    {
        _fastled->setAnimationId(id);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationIdRead");
        updated = false;
    }

    unsigned char option;
    rc = usbfaceFastledAnimationOptionRead(dev, _channel, _fastled->getAnimationId(), &option);
    if (USBFACE_SUCCESS == rc)
    {
        _fastled->setOption(option);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationIdRead");
        updated = false;
    }

    unsigned char running;
    rc = usbfaceFastledStateRead(dev, _channel, &running);
    if (USBFACE_SUCCESS == rc)
    {
        _fastled->setRunning(running);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledStateRead");
        updated = false;
    }

    unsigned char fps;
    rc = usbfaceFastledFPSRead(dev, _channel, _fastled->getAnimationId(), &fps);
    if (USBFACE_SUCCESS == rc)
    {
    	_fastled->setFps(fps);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledFPSRead");
        updated = false;
    }

    unsigned char autoStart;
    rc = usbfaceFastledAutostartRead(dev, _channel, &autoStart);
    if (USBFACE_SUCCESS == rc)
    {
        _fastled->setAutoStart(autoStart);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAutostartRead");
        updated = false;
    }

    unsigned char snsIdx;
    rc = usbfaceFastledSnsIdRead(dev, _channel, _fastled->getAnimationId(), &snsIdx);
    if (USBFACE_SUCCESS == rc)
    {
        _fastled->setSensorIndex(snsIdx);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledSnsIdRead");
        updated = false;
    }

    unsigned char colors[3];
    rc = usbfaceFastledColorRead(dev, _channel, _fastled->getAnimationId(), 0, colors);
    if (USBFACE_SUCCESS == rc)
    {
        QColor color;
        color.setHsv(round((360.0/256.0) * colors[0]), colors[1], colors[2]);
        _fastled->setColor1(color);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorRead");
        updated = false;
    }

    rc = usbfaceFastledColorRead(dev, _channel, _fastled->getAnimationId(), 1, colors);
    if (USBFACE_SUCCESS == rc)
    {
        QColor color;
        color.setHsv(round((360.0/256.0) * colors[0]), colors[1], colors[2]);
        _fastled->setColor2(color);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorRead");
        updated = false;
    }

    if (updated)
    {
        postEvent(CommandEvents::EventAllDataUpdated);
    }
}

CommandUpdateFastLedState::CommandUpdateFastLedState(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandUpdateFastLedState::_exec()
{
    hid_device *dev = _co->getDevHandle();
    bool updatedState = true;
    bool updateAll = true;

    unsigned char id;
    int rc = usbfaceFastledAnimationIdRead(dev, _channel, &id);
    if (USBFACE_SUCCESS == rc)
    {
        QMutexLocker l(_fastled->mutex());
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationIdRead");
        updateAll = false;
    }

    if (updateAll)
    {
        bool updated = true;

        unsigned char option;
        int rc = usbfaceFastledAnimationOptionRead(dev, _channel, _fastled->getAnimationId(), &option);
        if (USBFACE_SUCCESS == rc)
        {
            _fastled->setOption(option);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationIdRead");
            updated = false;
        }

        unsigned char running;
        rc = usbfaceFastledStateRead(dev, _channel, &running);
        if (USBFACE_SUCCESS == rc)
        {
            _fastled->setRunning(running);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledStateRead");
            updated = false;
        }

        unsigned char fps;
        rc = usbfaceFastledFPSRead(dev, _channel, _fastled->getAnimationId(), &fps);
        if (USBFACE_SUCCESS == rc)
        {
            _fastled->setFps(fps);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledFPSRead");
            updated = false;
        }

        unsigned char autoStart;
        rc = usbfaceFastledAutostartRead(dev, _channel, &autoStart);
        if (USBFACE_SUCCESS == rc)
        {
            _fastled->setAutoStart(autoStart);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledAutostartRead");
            updated = false;
        }

        unsigned char snsIdx;
        rc = usbfaceFastledSnsIdRead(dev, _channel, _fastled->getAnimationId(), &snsIdx);
        if (USBFACE_SUCCESS == rc)
        {
            _fastled->setSensorIndex(snsIdx);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledSnsIdRead");
            updated = false;
        }

        unsigned char colors[3];
        rc = usbfaceFastledColorRead(dev, _channel, _fastled->getAnimationId(), 0, colors);
        if (USBFACE_SUCCESS == rc)
        {
            QColor color;
            color.setHsv(round((360.0/256.0) * colors[0]), colors[1], colors[2]);
            //qDebug() << __PRETTY_FUNCTION__ << " " << colors[0] << " " << colors[1] << " " << colors[2];
            //qDebug() << __PRETTY_FUNCTION__ << " " << color.hue() << " " << color.saturation() << " " << color.value();
            _fastled->setColor1(color);
            //qDebug() << __PRETTY_FUNCTION__ << " " << _fastled->getColor1();
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorRead");
            updated = false;
        }

        rc = usbfaceFastledColorRead(dev, _channel, _fastled->getAnimationId(), 1, colors);
        if (USBFACE_SUCCESS == rc)
        {
            QColor color;
            color.setHsv(round((360.0/256.0) * colors[0]), colors[1], colors[2]);
            //qDebug() << __PRETTY_FUNCTION__ << " " << colors[0] << " " << colors[1] << " " << colors[2];
            //qDebug() << __PRETTY_FUNCTION__ << " " << color.hue() << " " << color.saturation() << " " << color.value();
            _fastled->setColor2(color);
            //qDebug() << __PRETTY_FUNCTION__ << " " << _fastled->getColor2();
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorRead");
            updated = false;
        }

        if (updated)
        {
            postEvent(CommandEvents::EventAllDataUpdated);
        }
    }

    if (!updateAll)
    {
        unsigned char running;
        rc = usbfaceFastledStateRead(dev, _channel, &running);
        if (USBFACE_SUCCESS == rc)
        {
            updatedState = (_fastled->isRunning() != running);
            _fastled->setRunning(running);
        }
        else
        {
            postUsbCommunicationErrorEvent(rc, "usbfaceFastledStateRead");
            updatedState = false;
        }

        if (updatedState)
        {
            postEvent(CommandEvents::EventValueUpdated);
        }
    }
}

CommandSetFastLedAnimationId::CommandSetFastLedAnimationId(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedAnimationId::_exec()
{
    hid_device *dev = _co->getDevHandle();
    unsigned char id;

    _fastled->mutex()->lock();
    id = _fastled->getAnimationId();
    _fastled->mutex()->unlock();

    int rc = usbfaceFastledAnimationIdWrite(dev, _channel, id);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationIdWrite");
    }
}

CommandSetFastLedAnimationOption::CommandSetFastLedAnimationOption(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedAnimationOption::_exec()
{
    hid_device *dev = _co->getDevHandle();
    unsigned char option;

    _fastled->mutex()->lock();
    option = _fastled->getOption();
    _fastled->mutex()->unlock();

    int rc = usbfaceFastledAnimationOptionWrite(dev, _channel, _fastled->getAnimationId(), option);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAnimationOptionWrite");
    }
}

CommandSetFastLedAnimationFPS::CommandSetFastLedAnimationFPS(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedAnimationFPS::_exec()
{
    hid_device *dev = _co->getDevHandle();
    unsigned char fps;

    _fastled->mutex()->lock();
    fps = _fastled->getFps();
    _fastled->mutex()->unlock();

    int rc = usbfaceFastledFPSWrite(dev, _channel, _fastled->getAnimationId(), fps);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledFPSWrite");
    }
}

CommandSetFastLedAnimationColor::CommandSetFastLedAnimationColor(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedAnimationColor::_exec()
{
    hid_device *dev = _co->getDevHandle();

    QColor color1;
    unsigned char colors[3];

    _fastled->mutex()->lock();
    color1 = _fastled->getColor1();
    _fastled->mutex()->unlock();

    colors[0] = round((256.0/360.0) * color1.hue());
    colors[1] = color1.saturation();
    colors[2] = color1.value();

    qDebug() << __PRETTY_FUNCTION__ << " " << colors[0] << " " << colors[1] << " " << colors[2];

    int rc = usbfaceFastledColorWrite(dev, _channel, _fastled->getAnimationId(), 0, colors);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorWrite");
    }

    _fastled->mutex()->lock();
    color1 = _fastled->getColor2();
    _fastled->mutex()->unlock();

    colors[0] = round((256.0/360.0) * color1.hue());
    colors[1] = color1.saturation();
    colors[2] = color1.value();


    qDebug() << __PRETTY_FUNCTION__ << " " << colors[0] << " " << colors[1] << " " << colors[2];

    rc = usbfaceFastledColorWrite(dev, _channel, _fastled->getAnimationId(), 1, colors);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledColorWrite");
    }
}

CommandSetFastLedState::CommandSetFastLedState(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedState::_exec()
{
    hid_device *dev = _co->getDevHandle();
    unsigned char running;

    _fastled->mutex()->lock();
    running = _fastled->isRunning();
    _fastled->mutex()->unlock();

    int rc = usbfaceFastledStateWrite(dev, _channel, running);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledStateWrite");
    }
}

CommandSetFastLedConfiguration::CommandSetFastLedConfiguration(DataFastLed *fastled) : CommandUpdateFastLed(fastled)
{
    setName(__func__);
}

void CommandSetFastLedConfiguration::_exec()
{
    hid_device *dev = _co->getDevHandle();

    unsigned char autoStart;
    unsigned char snsIdx;

    _fastled->mutex()->lock();
    autoStart = _fastled->isAutoStart();
    snsIdx = _fastled->getSensorIndex();

    _fastled->mutex()->unlock();

    int rc = usbfaceFastledAutostartWrite(dev, _channel, autoStart);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledAutostartWrite");
    }

    rc = usbfaceFastledSnsIdWrite(dev, _channel, _fastled->getAnimationId(), snsIdx);
    if (USBFACE_SUCCESS != rc)
    {
        postUsbCommunicationErrorEvent(rc, "usbfaceFastledSnsIdWrite");
    }
}

/*
 * ----------------- powermeter ----------------------------------
 */

CommandUpdatePowerMeter::CommandUpdatePowerMeter(DataPowerMeter *power)
    : CommandManipulateAChannelNotifyACallerUsbDevice(power, power->channel()), _power(power)
{
    setName(__func__);
}

void CommandUpdatePowerMeter::_exec()
{
    qDebug() << "CommandUpdatePowerMeter " << _power->fullName();

    bool updated = true;
    hid_device *dev = _co->getDevHandle();

    unsigned int milliwatt;
    int rc = usbfacePowerMeterPowerRead(dev, _channel, &milliwatt);
    if (USBFACE_SUCCESS == rc)
    {
        _power->setPower_mW(milliwatt);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfacePowerMeterPowerRead");
        updated = false;
    }

    unsigned int millivolt;
    rc = usbfacePowerMeterLoadRead(dev, _channel, &millivolt);
    if (USBFACE_SUCCESS == rc)
    {
        _power->setLoad_mV(millivolt);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfacePowerMeterLoadRead");
        updated = false;
    }

    unsigned int milliampere;
    rc = usbfacePowerMeterCurrentRead(dev, _channel, &milliampere);
    if (USBFACE_SUCCESS == rc)
    {
        _power->setCurrent_mA(milliampere);
    }
    else
    {
        postUsbCommunicationErrorEvent(rc, "usbfacePowerMeterLoadRead");
        updated = false;
    }

    if (updated)
    {
        postEvent(CommandEvents::EventValueUpdated);
    }
}
