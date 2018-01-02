#include "mainwindow.h"
#include "hid_device.h"
#include "command_consumer.h"
#include "command_queue.h"
#include "usb_connection.h"
#include "data_sensor.h"
#include "series_sensor.h"
#include "data_fan.h"
#include "series_fan.h"
#include "data_fan_out.h"
#include "series_fanout.h"
#include "data_fastled.h"
#include "data_properties.h"
#include "data_powermeter.h"
#include "series_powermeter.h"
#include "connection_watcher.h"
#include "value_updater.h"
#include "value_updater_test.h"
#include "widget_fan_container_form.h"
#include "widget_fan_form.h"
#include "widget_fan_show_form.h"
#include "widget_sensor_form.h"
#include "widget_fanout_form.h"
#include "widget_fastled_form.h"
#include "widget_sensor_container_form.h"
#include "widget_fanout_container_form.h"
#include "widget_fastled_container_form.h"
#include "widget_deviceinformation_form.h"
#include "widget_powermeter_form.h"
#include "widget_powermeter_container_form.h"

#include <QApplication>
#include <QTabWidget>
#include <QLayout>
#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <memory>

int main(int argc, char *argv[])
{
    std::vector<std::shared_ptr<DataFan>> dataFans;
    std::vector<std::shared_ptr<DataSensor>> dataSensors;

    QApplication a(argc, argv);
    MainWindow w;

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        qDebug() << "I couldn't detect any system tray on this system";
    }

    QApplication::setQuitOnLastWindowClosed(false);

	ConnectionWatcher watcher;
	ValueUpdater updater;
    ValueUpdaterTest tester;

    std::shared_ptr<DataDeviceProperties> properties = std::make_shared<DataDeviceProperties>();
    watcher.registerPropertyDataObject(properties);
    // properties has no values that need an update on a regular basis, so no registration with updater

    WidgetDeviceInformationForm *devinfo = new WidgetDeviceInformationForm(properties);
    w.addWidget(MainWindow::Feature::Device, devinfo, "Device");

    //WidgetFastLedContainerForm *fastled = new WidgetFastLedContainerForm();
    //w.addWidget(fastled, "LED stripes");

    std::shared_ptr<DataFastLed> dataFastLed0 = std::make_shared<DataFastLed>(0);
    WidgetFastLEDForm *fled0 = new WidgetFastLEDForm(dataFastLed0);
    QObject::connect(&w, SIGNAL(currentChanged(int)), fled0, SLOT(on_currentTabChanged(int)));
    QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), fled0, SLOT(on_supportedFunctionsUpdated(int)));
    //fastled->addWidgetFastLed(fled0);
    w.addWidget(MainWindow::Feature::LEDStripe, fled0, dataFastLed0->name());

    std::shared_ptr<DataFastLed> dataFastLed1 = std::make_shared<DataFastLed>(1);
    WidgetFastLEDForm *fled1 = new WidgetFastLEDForm(dataFastLed1);
    QObject::connect(&w, SIGNAL(currentChanged(int)), fled1, SLOT(on_currentTabChanged(int)));
    QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), fled1, SLOT(on_supportedFunctionsUpdated(int)));
    //fastled->addWidgetFastLed(fled1);
    w.addWidget(MainWindow::Feature::LEDStripe, fled1, dataFastLed1->name());



    WidgetSensorContainerForm *sensorListWidget = new WidgetSensorContainerForm();

    for (int i = 0; i < MAX_SNS; i++)
    {
        std::shared_ptr<DataSensor> sensor = std::make_shared<DataSensor>(i);
        watcher.registerDataObject(sensor);
        updater.registerDataObject(sensor);



        tester.registerDataObject(sensor);



        WidgetSensorForm *wsf = new WidgetSensorForm(sensor);
        sensorListWidget->addWidgetSensor(wsf, i != MAX_SNS - 1);

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wsf, SLOT(on_supportedFunctionsUpdated(int)));

        dataSensors.push_back(sensor);
    }

    w.addWidget(MainWindow::Feature::Sensor, sensorListWidget, "Sensors");



    fled0->setDataSensors(dataSensors);
    fled1->setDataSensors(dataSensors);


    WidgetFanContainerForm *fanContainer = new WidgetFanContainerForm();

    for (int i = 0; i < MAX_FANS; i++)
    {
        std::shared_ptr<DataFan> fan = std::make_shared<DataFan>(i);
        watcher.registerDataObject(fan);
        updater.registerDataObject(fan);
        dataFans.push_back(fan);



        tester.registerDataObject(fan);



        WidgetFanForm *wf = new WidgetFanForm(fan);
        wf->setDataSensors(dataSensors);

        w.addWidget(MainWindow::Feature::Fan, wf, fan->name());

        QObject::connect(&w, SIGNAL(currentChanged(int)), wf, SLOT(on_currentTabChanged(int)));
        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wf, SLOT(on_supportedFunctionsUpdated(int)));

        WidgetFanShowForm *wsf = new WidgetFanShowForm(fan);
        wsf->setDataSensors(dataSensors);

        fanContainer->addWidgetFan(wsf, i != MAX_FANS - 1);
    }

    w.setWidget(MainWindow::Feature::Fan, fanContainer);



    //WidgetFanoutContainerForm *fanoutListWidget = new WidgetFanoutContainerForm();

    for (int i = 0; i < MAX_FAN_OUTS; i++)
    {
        std::shared_ptr<DataFanOut> fanout = std::make_shared<DataFanOut>(i);

        watcher.registerDataObject(fanout);
        updater.registerDataObject(fanout);

        WidgetFanOutForm *wfof = new WidgetFanOutForm(fanout);
        wfof->setDataFans(dataFans);
        //fanoutListWidget->addWidgetFanout(wfof);
        w.addWidget(MainWindow::Feature::FanOutput, wfof, fanout->name());

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wfof, SLOT(on_supportedFunctionsUpdated(int)));
    }

    //fanoutLayout->addStretch(2);
    //w.addWidget(fanoutListWidget, "Fan Out");


    //WidgetPowerMeterContainerForm *powermeterListWidget = new WidgetPowerMeterContainerForm();

    for (int i = 0; i < MAX_POWERMETER; i++)
    {
        std::shared_ptr<DataPowerMeter> powermeter = std::make_shared<DataPowerMeter>(i);

        watcher.registerDataObject(powermeter);
        updater.registerDataObject(powermeter);

        WidgetPowerMeterForm *wfof = new WidgetPowerMeterForm(powermeter);
        //powermeterListWidget->addWidgetPowerMeter(wfof);
        w.addWidget(MainWindow::Feature::PowerMeter, wfof, powermeter->name());

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wfof, SLOT(on_supportedFunctionsUpdated(int)));
    }

    //w.addWidget(powermeterListWidget, "Power");



    std::shared_ptr<UsbConnection> usbConnection = std::make_shared<UsbConnection>();


    UsbCommandConsumer consumer(usbConnection);
    //UsbCommandWorkerConsumer consumer(usbConnection);
    consumer.start();



    QObject::connect(usbConnection.get(), SIGNAL(signalConnectionChanged(bool)), &watcher, SLOT(on_usbConectionChanged(bool)));
    QObject::connect(usbConnection.get(), SIGNAL(signalConnectionChanged(bool)), &updater, SLOT(on_usbConectionChanged(bool)));
    QObject::connect(usbConnection.get(), SIGNAL(signalConnectionChanged(bool)), &w, SLOT(on_usbConectionChanged(bool)));

    QObject::connect(&w, SIGNAL(signalResyncToDevice()), &watcher, SLOT(on_requestResync()));
    QObject::connect(&w, SIGNAL(signalSettingsSave()), devinfo, SLOT(on_settingsSave()));
    QObject::connect(&w, SIGNAL(signalSettingsLoad()), devinfo, SLOT(on_settingsLoad()));
    QObject::connect(&w, SIGNAL(signalSettingsClear()), devinfo, SLOT(on_settingsClear()));

    w.readSettings();
    w.show();

    sensorListWidget->readSettings();

    //tester.startUpdates();

    usbConnection->connectToDevice();

    int rc = a.exec();

    w.saveSettings();

    consumer.requestInterruption();
    CommandQueueInstance().requestInterruption();
    consumer.wait();

    tester.stopUpdates();
    updater.stopUpdates();
    usbConnection->disconnectFromDevice();

    return rc;
}
