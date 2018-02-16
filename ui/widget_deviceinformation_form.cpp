#include "widget_deviceinformation_form.h"
#include "ui_widget_deviceinformation_form.h"
#include "data_properties.h"
#include <QtCore/QDebug>

WidgetDeviceInformationForm::WidgetDeviceInformationForm(std::shared_ptr<DataDeviceProperties> properties, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetDeviceInformationForm),
    _properties(properties)
{
    ui->setupUi(this);

    showSupportedFunctions(SUPPORTED_NONE);

    connect(_properties.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_properties.get(), SIGNAL(signalFinished()), this, SLOT(on_commandFinished()));
    connect(_properties.get(), SIGNAL(signalUsbCommunicationFailed(int, QString)), this, SLOT(on_usbCommunicationFailed(int,QString)));
}

WidgetDeviceInformationForm::~WidgetDeviceInformationForm()
{
    delete ui;
}

void WidgetDeviceInformationForm::on_settingsClear()
{
    if (ui->pbSettingsClear->isEnabled())
        on_pbSettingsClear_clicked();
}

void WidgetDeviceInformationForm::on_settingsLoad()
{
    if (ui->pbSettingsLoad->isEnabled())
        on_pbSettingsLoad_clicked();
}

void WidgetDeviceInformationForm::on_settingsSave()
{
    if (ui->pbSettingsSave->isEnabled())
        on_pbSettingsSave_clicked();
}

void WidgetDeviceInformationForm::showSupportedFunctions(int supportedFunctions)
{
    ui->fSensorAnalog->setFeatureIsSupported(supportedFunctions & SUPPORTED_SNS_ANALOG);
    ui->fSensorDigital->setFeatureIsSupported(supportedFunctions & SUPPORTED_SNS_I2C);
    ui->fSensorPowerMeter->setFeatureIsSupported(supportedFunctions & SUPPORTED_SNS_POWER);

    ui->fFCFuzzy->setFeatureIsSupported(supportedFunctions & SUPPORTED_FANMODE_FUZZY);
    ui->fFCLinear->setFeatureIsSupported(supportedFunctions & SUPPORTED_FANMODE_LINEAR);
    ui->fFCLinearTripPoints->setFeatureIsSupported(supportedFunctions & SUPPORTED_FANMODE_LIN_TP);
    ui->fFCTripPoints->setFeatureIsSupported(supportedFunctions & SUPPORTED_FANMODE_TP);
    ui->fFCPI->setFeatureIsSupported(supportedFunctions & SUPPORTED_FANMODE_PI);

    ui->fHWFanOutput->setFeatureIsSupported(supportedFunctions & SUPPORTED_FAN_OUT);
    ui->fHWLedStripes->setFeatureIsSupported(supportedFunctions & SUPPORTED_LED_FASTLED);

    ui->fOtherPIControllerDebug->setFeatureIsSupported(supportedFunctions & SUPPORTED_CTRL_DEBUG);
    ui->fOtherEEMEMUpload->setFeatureIsSupported(supportedFunctions & SUPPORTED_EEPROM_UPDOWNLD);
    ui->fOtherUSBWatchdog->setFeatureIsSupported(supportedFunctions & SUPPORTED_USBCOMM_WATCHDOG);
}

void WidgetDeviceInformationForm::on_dataUpdated()
{
    ui->leDevice->setText(_properties->getDeviceVendorString());
    ui->leFirmwareVersion->setText(_properties->getFirmwareVersionString());
    ui->leProtocolVersion->setText(_properties->getVersionProtocolString());
    ui->leName->setText(_properties->getDeviceName());
    showSupportedFunctions(_properties->getSupportedFunctions());
}

void WidgetDeviceInformationForm::on_commandFinished()
{
    qDebug() << __PRETTY_FUNCTION__;
    enableSettingsButtons(true);
}

void WidgetDeviceInformationForm::on_usbCommunicationFailed(int errorCode, const QString &errorMsg)
{
    Q_UNUSED(errorMsg);
    Q_UNUSED(errorCode);
    qDebug() << __PRETTY_FUNCTION__;
    enableSettingsButtons(true);
}

void WidgetDeviceInformationForm::enableSettingsButtons(bool enabled)
{
    ui->pbSettingsSave->setEnabled(enabled);
    ui->pbSettingsLoad->setEnabled(enabled);
    ui->pbSettingsClear->setEnabled(enabled);
}

void WidgetDeviceInformationForm::on_pbSettingsSave_clicked()
{
    qDebug() << __PRETTY_FUNCTION__;
    enableSettingsButtons(false);
    _properties->settingsSave();
}

void WidgetDeviceInformationForm::on_pbSettingsLoad_clicked()
{
    enableSettingsButtons(false);
    _properties->settingsLoad();
}

void WidgetDeviceInformationForm::on_pbSettingsClear_clicked()
{
    enableSettingsButtons(false);
    _properties->settingsClear();
}

void WidgetDeviceInformationForm::on_leName_editingFinished()
{
    _properties->updateDeviceName(ui->leName->text());
}
