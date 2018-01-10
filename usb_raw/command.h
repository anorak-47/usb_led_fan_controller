#ifndef COMMAND_H
#define COMMAND_H

#include "command_event.h"
#include <memory>

class QObject;
class DataSensor;
class DataFan;
class DataFanOut;
class DataFastLed;
class DataPowerMeter;
class DataDeviceProperties;
class ConnectionObject;
class UsbConnection;

class Command
{
public:
    Command();
    virtual ~Command();
    virtual void exec(std::shared_ptr<ConnectionObject> &co) = 0;

    void setName(QString const &name);
    QString getName() const;

private:
    QString _name;
};

class CommandNotifyACaller : public Command
{
public:
	CommandNotifyACaller(QObject *caller);

	virtual void postEvent(CommandEvents eventType);
	virtual void postUsbCommunicationErrorEvent(int errorCode, QString const &msg);

protected:
	QObject *_caller;
};

class CommandNotifyACallerUsbDevice : public CommandNotifyACaller
{
public:
	CommandNotifyACallerUsbDevice(QObject *caller);
    virtual void exec(std::shared_ptr<ConnectionObject> &co) final;
protected:
    virtual void _exec() = 0;
    std::shared_ptr<UsbConnection> _co;
};

class CommandUpdateDeviceProperties : public CommandNotifyACallerUsbDevice
{
public:
	CommandUpdateDeviceProperties(DataDeviceProperties *caller);
	virtual void _exec() override;

protected:
	DataDeviceProperties *_properties;
};

class CommandSettingsSave : public CommandNotifyACallerUsbDevice
{
public:
    CommandSettingsSave(DataDeviceProperties *caller);
    virtual void _exec() override;

protected:
    DataDeviceProperties *_properties;
};

class CommandSettingsLoad : public CommandNotifyACallerUsbDevice
{
public:
    CommandSettingsLoad(bool clearSettings, DataDeviceProperties *caller);
    virtual void _exec() override;

protected:
    DataDeviceProperties *_properties;
    bool _clearSettings;
};

class CommandManipulateAChannel : public Command
{
public:
	CommandManipulateAChannel(unsigned int channel);
	unsigned int getChannel() const;
protected:
	unsigned int _channel;
};

class CommandManipulateAChannelNotifyACaller : public CommandManipulateAChannel
{
public:
	CommandManipulateAChannelNotifyACaller(QObject *caller, unsigned int channel);

	virtual void postEvent(CommandEvents eventType);
	virtual void postUsbCommunicationErrorEvent(int errorCode, QString const &msg);

protected:
	QObject *_caller;
};

class CommandManipulateAChannelNotifyACallerUsbDevice : public CommandManipulateAChannelNotifyACaller
{
public:
    CommandManipulateAChannelNotifyACallerUsbDevice(QObject *caller, unsigned int channel);
    virtual void exec(std::shared_ptr<ConnectionObject> &co) final;
protected:
    virtual void _exec() = 0;
    std::shared_ptr<UsbConnection> _co;
};

/*
 * ----------------- sensor ----------------------------------
 */

class CommandUpdateSensor : public CommandManipulateAChannelNotifyACallerUsbDevice
{
public:
	CommandUpdateSensor(DataSensor* sensor);

protected:
    virtual void _exec() override;
	DataSensor *_sensor;
};

class CommandUpdateSensorValue : public CommandUpdateSensor
{
public:
    CommandUpdateSensorValue(DataSensor *sensor);
protected:
    void _exec() override;
};

class CommandSetSensorType : public CommandUpdateSensor
{
public:
    CommandSetSensorType(DataSensor* sensor);
protected:
    void _exec() override;
};

class CommandSetSensorValue : public CommandUpdateSensor
{
public:
    CommandSetSensorValue(DataSensor* sensor);
protected:
    void _exec() override;
};

/*
 * ----------------- fanout ----------------------------------
 */

class CommandUpdateFanOut : public CommandManipulateAChannelNotifyACallerUsbDevice
{
public:
	CommandUpdateFanOut(DataFanOut* fanout);

protected:
    virtual void _exec() override;
	DataFanOut *_fanout;
};

class CommandUpdateFanOutValue : public CommandUpdateFanOut
{
public:
    CommandUpdateFanOutValue(DataFanOut *fanout);
protected:
    void _exec() override;
};

class CommandUpdateFanOutStallDetection : public CommandUpdateFanOut
{
public:
    CommandUpdateFanOutStallDetection(DataFanOut *fanout);
protected:
    void _exec() override;
};

class CommandSetFanOutMode : public CommandUpdateFanOut
{
public:
    CommandSetFanOutMode(DataFanOut* fanout);
protected:
    void _exec() override;
};

/*
 * ----------------- fan ----------------------------------
 */

class CommandUpdateFan : public CommandManipulateAChannelNotifyACallerUsbDevice
{
public:
	CommandUpdateFan(DataFan* fan);

protected:
    virtual void _exec() override;
	DataFan *_fan;
};

class CommandUpdateFanValue : public CommandUpdateFan
{
public:
    CommandUpdateFanValue(DataFan *fan);
protected:
    void _exec() override;
};

class CommandSetFanConfig : public CommandUpdateFan
{
public:
    CommandSetFanConfig(DataFan* fan);
protected:
    void _exec() override;
};

class CommandSetFanFixedDutyController : public CommandUpdateFan
{
public:
	CommandSetFanFixedDutyController(DataFan* fan);
protected:
    void _exec() override;
};

class CommandSetFanLinearController : public CommandUpdateFan
{
public:
	CommandSetFanLinearController(DataFan* fan);
protected:
    void _exec() override;
};

class CommandSetFanTripPointController : public CommandUpdateFan
{
public:
	CommandSetFanTripPointController(DataFan* fan);
protected:
    void _exec() override;
};

class CommandSetFanPiController : public CommandUpdateFan
{
public:
	CommandSetFanPiController(DataFan* fan);
protected:
    void _exec() override;
};

/*
 * ----------------- fastled ----------------------------------
 */

class CommandUpdateFastLed : public CommandManipulateAChannelNotifyACallerUsbDevice
{
public:
	CommandUpdateFastLed(DataFastLed *caller);

protected:
	virtual void _exec() override;
	DataFastLed *_fastled;
};

class CommandUpdateFastLedState : public CommandUpdateFastLed
{
public:
	CommandUpdateFastLedState(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedAnimationId : public CommandUpdateFastLed
{
public:
	CommandSetFastLedAnimationId(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedAnimationOption : public CommandUpdateFastLed
{
public:
    CommandSetFastLedAnimationOption(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedAnimationFPS : public CommandUpdateFastLed
{
public:
    CommandSetFastLedAnimationFPS(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedAnimationColor : public CommandUpdateFastLed
{
public:
    CommandSetFastLedAnimationColor(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedState : public CommandUpdateFastLed
{
public:
	CommandSetFastLedState(DataFastLed* fan);
protected:
    void _exec() override;
};

class CommandSetFastLedConfiguration : public CommandUpdateFastLed
{
public:
	CommandSetFastLedConfiguration(DataFastLed* fastled);
protected:
    void _exec() override;
};

/*
 * ----------------- power meter ----------------------------------
 */

class CommandUpdatePowerMeter : public CommandManipulateAChannelNotifyACallerUsbDevice
{
public:
    CommandUpdatePowerMeter(DataPowerMeter *caller);

protected:
    virtual void _exec() override;
    DataPowerMeter *_power;
};

#endif // CHANGECOMMAND_H
