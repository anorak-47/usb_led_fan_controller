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


    WidgetSensorContainerForm *sensorListWidget = new WidgetSensorContainerForm();

    for (int i = 0; i < MAX_SNS; i++)
    {
        std::shared_ptr<DataSensor> sensor = std::make_shared<DataSensor>(i);
        watcher.registerDataObject(sensor);
        updater.registerDataObject(sensor);



        tester.registerDataObject(sensor);



        WidgetSensorForm *wsf = new WidgetSensorForm(sensor);
        sensorListWidget->addWidgetSensor(wsf, i != MAX_SNS - 1);
        QObject::connect(&w, SIGNAL(currentChanged(int)), wsf, SLOT(on_currentTabChanged(int)));

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wsf, SLOT(on_supportedFunctionsUpdated(int)));

        dataSensors.push_back(sensor);
    }

    w.addWidget(MainWindow::Feature::Sensor, sensorListWidget, "Sensors");




    WidgetFanContainerForm *fanContainer = new WidgetFanContainerForm();
    QObject::connect(&w, SIGNAL(currentChanged(int)), fanContainer, SLOT(on_currentTabChanged(int)));

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
        QObject::connect(&w, SIGNAL(currentChanged(int)), wsf, SLOT(on_currentTabChanged(int)));

        fanContainer->addWidgetFan(wsf, i != MAX_FANS - 1);
    }

    w.setWidget(MainWindow::Feature::Fan, fanContainer);



    //WidgetFanoutContainerForm *fanoutListWidget = new WidgetFanoutContainerForm();

    for (int i = 0; i < MAX_FAN_OUTS; i++)
    {
        std::shared_ptr<DataFanOut> fanout = std::make_shared<DataFanOut>(i);

        watcher.registerDataObject(fanout);
        updater.registerDataObject(fanout);



        tester.registerDataObject(fanout);



        WidgetFanOutForm *wfof = new WidgetFanOutForm(fanout);
        wfof->setDataFans(dataFans);
        //fanoutListWidget->addWidgetFanout(wfof);
        QObject::connect(&w, SIGNAL(currentChanged(int)), wfof, SLOT(on_currentTabChanged(int)));

        w.addWidget(MainWindow::Feature::FanOutput, wfof, fanout->name());        

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wfof, SLOT(on_supportedFunctionsUpdated(int)));
    }

    //fanoutLayout->addStretch(2);
    //w.addWidget(fanoutListWidget, "Fan Out");




    //WidgetFastLedContainerForm *fastled = new WidgetFastLedContainerForm();
    //w.addWidget(fastled, "LED stripes");


    for (int i = 0; i < MAX_LED_STRIPES; i++)
    {
        std::shared_ptr<DataFastLed> dataFastLed = std::make_shared<DataFastLed>(i);

        watcher.registerDataObject(dataFastLed);

        WidgetFastLEDForm *fled = new WidgetFastLEDForm(dataFastLed);
        fled->setDataSensors(dataSensors);
        QObject::connect(&w, SIGNAL(currentChanged(int)), fled, SLOT(on_currentTabChanged(int)));
        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), fled, SLOT(on_supportedFunctionsUpdated(int)));
        //fastled->addWidgetFastLed(fled);
        w.addWidget(MainWindow::Feature::LEDStripe, fled, dataFastLed->name());
    }



    //WidgetPowerMeterContainerForm *powermeterListWidget = new WidgetPowerMeterContainerForm();

    for (int i = 0; i < MAX_POWERMETER; i++)
    {
        std::shared_ptr<DataPowerMeter> powermeter = std::make_shared<DataPowerMeter>(i);

        watcher.registerDataObject(powermeter);
        updater.registerDataObject(powermeter);



        tester.registerDataObject(powermeter);




        WidgetPowerMeterForm *wpmf = new WidgetPowerMeterForm(powermeter);
        //powermeterListWidget->addWidgetPowerMeter(wfof);
        QObject::connect(&w, SIGNAL(currentChanged(int)), wpmf, SLOT(on_currentTabChanged(int)));

        w.addWidget(MainWindow::Feature::PowerMeter, wpmf, powermeter->name());

        QObject::connect(properties.get(), SIGNAL(signalSupportedFunctionsUpdated(int)), wpmf, SLOT(on_supportedFunctionsUpdated(int)));
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




    tester.startUpdates();




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
