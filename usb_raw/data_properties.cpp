#include "data_properties.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>

DataDeviceProperties::DataDeviceProperties()
{
}

DataDeviceProperties::~DataDeviceProperties()
{
}

QString DataDeviceProperties::name() const
{
    return "Properties";
}

QString DataDeviceProperties::fullName() const
{
    return "Properties";
}

QString DataDeviceProperties::description() const
{
    return "Properties";
}

void DataDeviceProperties::update()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandUpdateDeviceProperties>(new CommandUpdateDeviceProperties(this))));
}

int DataDeviceProperties::getNrOfFanOuts() const
{
    return _nrOf_fan_outs;
}

void DataDeviceProperties::setNrOfFanOuts(int nrOfFanOuts)
{
    _nrOf_fan_outs = nrOfFanOuts;
}

int DataDeviceProperties::getNrOfFans() const
{
    return _nrOf_fans;
}

void DataDeviceProperties::setNrOfFans(int nrOfFans)
{
    _nrOf_fans = nrOfFans;
}

int DataDeviceProperties::getNrOfSensors() const
{
    return _nrOf_sensors;
}

void DataDeviceProperties::setNrOfSensors(int nrOfSensors)
{
    _nrOf_sensors = nrOfSensors;
}

void DataDeviceProperties::updateValues()
{
}

void DataDeviceProperties::settingsSave()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSettingsSave>(new CommandSettingsSave(this))));
}

void DataDeviceProperties::settingsLoad()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSettingsLoad>(new CommandSettingsLoad(false, this))));
}

void DataDeviceProperties::settingsClear()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSettingsLoad>(new CommandSettingsLoad(true, this))));
}

int DataDeviceProperties::getSupportedFunctions() const
{
    QMutexLocker l(_mutex);
    return _funcs;
}

void DataDeviceProperties::setSupportedFunctions(int funcs)
{
    QMutexLocker l(_mutex);
    _funcs = funcs;
}

bool DataDeviceProperties::handleEvent(CommandEvent *event)
{
    if (event->type() == (QEvent::Type)CommandEvents::EventPropertiesUpdated)
    {
        qDebug() << " EventPropertiesUpdated event";

        emit signalChanged();
        emit signalSupportedFunctionsUpdated(_funcs);
        return true;
    }
    if (event->type() == (QEvent::Type)CommandEvents::EventCommandFinished)
    {
        qDebug() << " EventCommandFinished event";

        emit signalFinished();
        return true;
    }

    return false;
}

QString DataDeviceProperties::getDeviceDevice() const
{
    return _device_device;
}

void DataDeviceProperties::setDeviceDevice(const QString &device_device)
{
    _device_device = device_device;
}

QString DataDeviceProperties::getDeviceVendorString() const
{
    return QString("%1 - %2").arg(_device_device).arg(_device_vendor);
}

QString DataDeviceProperties::getFirmwareVersionString() const
{
    return QString("v%1.%2").arg(_version_firmware_major).arg(_version_firmware_minor);
}

QString DataDeviceProperties::getVersionProtocolString() const
{
    return QString::number(_version_protocol);
}

QString DataDeviceProperties::getDeviceVendor() const
{
    return _device_vendor;
}

void DataDeviceProperties::setDeviceVendor(const QString &device_vendor)
{
    _device_vendor = device_vendor;
}

QString DataDeviceProperties::getDeviceSerialNumber() const
{
    return _device_serial_number;
}

void DataDeviceProperties::setDeviceSerialNumber(const QString &device_serial_number)
{
    _device_serial_number = device_serial_number;
}

unsigned char DataDeviceProperties::getVersionProtocol() const
{
    return _version_protocol;
}

void DataDeviceProperties::setVersionProtocol(unsigned char version_protocol)
{
    _version_protocol = version_protocol;
}

unsigned char DataDeviceProperties::getVersionFirmwareMinor() const
{
    return _version_firmware_minor;
}

void DataDeviceProperties::setVersionFirmwareMinor(unsigned char version_firmware_minor)
{
    _version_firmware_minor = version_firmware_minor;
}

unsigned char DataDeviceProperties::getVersionFirmwareMajor() const
{
    return _version_firmware_major;
}

void DataDeviceProperties::setVersionFirmwareMajor(unsigned char version_firmware_major)
{
    _version_firmware_major = version_firmware_major;
}
