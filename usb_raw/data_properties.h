#pragma once

#include "data.h"
#include "types.h"
#include <QtCore/QObject>

class QMutex;

class DataDeviceProperties : public DataObject
{
    Q_OBJECT

public:
    DataDeviceProperties();
    virtual ~DataDeviceProperties();

    virtual QString name() const override;
    virtual QString fullName() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    void settingsSave();
    void settingsLoad();
    void settingsClear();

    void updateDeviceName(const QString &device_name);

    int getSupportedFunctions() const;
    void setSupportedFunctions(int funcs);

    int getNrOfFanOuts() const;
    void setNrOfFanOuts(int nrOfFanOuts);

    int getNrOfFans() const;
    void setNrOfFans(int nrOfFans);

    int getNrOfSensors() const;
    void setNrOfSensors(int nrOfSensors);

    unsigned char getVersionFirmwareMajor() const;
    void setVersionFirmwareMajor(unsigned char version_firmware_major);

    unsigned char getVersionFirmwareMinor() const;
    void setVersionFirmwareMinor(unsigned char version_firmware_minor);

    unsigned char getVersionProtocol() const;
    void setVersionProtocol(unsigned char version_protocol);

    QString getDeviceSerialNumber() const;
    void setDeviceSerialNumber(const QString &device_serial_number);

    QString getDeviceVendor() const;
    void setDeviceVendor(const QString &device_vendor);

    QString getDeviceDevice() const;
    void setDeviceDevice(const QString &device_device);

    QString getDeviceName() const;
    void setDeviceName(const QString &device_name);

    QString getDeviceVendorString() const;
    QString getFirmwareVersionString() const;
    QString getVersionProtocolString() const;

signals:
    void signalValueUpdated();
    void signalSupportedFunctionsUpdated(int supportedFunctions);
    void signalPropertiesUpdated();

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    int _funcs = SUPPORTED_NONE;

    int _nrOf_fans = 0;
    int _nrOf_sensors = 0;
    int _nrOf_fan_outs = 0;

    unsigned char _version_firmware_major = 0;
    unsigned char _version_firmware_minor = 0;
    unsigned char _version_protocol = 0;

    QString _device_serial_number;
    QString _device_vendor;
    QString _device_device;
    QString _device_name;
};
