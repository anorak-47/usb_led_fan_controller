#include "value_updater_test.h"
#include "data.h"
#include "data_sensor.h"
#include "data_fan.h"
#include "data_fan_out.h"
#include "data_powermeter.h"
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

ValueUpdaterTest::ValueUpdaterTest()
{
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
}

ValueUpdaterTest::~ValueUpdaterTest()
{
    delete _timer;
}

void ValueUpdaterTest::startUpdates()
{
    _timer->start(1000);
}

void ValueUpdaterTest::stopUpdates()
{
    _timer->stop();
}

void ValueUpdaterTest::registerDataObject(std::shared_ptr<DataObject> dataObject)
{
    dataObjects.push_back(dataObject);
}

void ValueUpdaterTest::postEvent(DataWithAChannel *caller, CommandEvents eventtype)
{
    //qDebug() << "ValueUpdaterTest: post event, channel: " << caller->channel();
    CommandEvent *event = new CommandEvent(eventtype, caller->channel());
    QCoreApplication::sendEvent(caller, event);
}

void ValueUpdaterTest::on_timeout()
{
    for (auto &dataObject : dataObjects)
    {
        std::shared_ptr<DataSensor> dataSensor = std::dynamic_pointer_cast<DataSensor>(dataObject);

        if (dataSensor)
        {
            //qDebug() << "ValueUpdaterTest: update sensor " << dataSensor->name();

            double value = dataSensor->data().value;
            value += (random() % 6) - 3;
            dataSensor->setValue(value);

            postEvent(dataSensor.get(), CommandEvents::EventValueUpdated);
            continue;
        }

        std::shared_ptr<DataFan> dataFan = std::dynamic_pointer_cast<DataFan>(dataObject);

        if (dataFan)
        {
            //qDebug() << "ValueUpdaterTest: update fan " << dataFan->name();

            double value = dataFan->getRPM();
            value += (random() % 100) - 50;
            dataFan->setRPM(value);

            dataFan->setScaledDuty(value / 5000.0 * 100.0 - 10.0);

            postEvent(dataFan.get(), CommandEvents::EventValueUpdated);
            continue;
        }

        std::shared_ptr<DataFanOut> dataFanOut = std::dynamic_pointer_cast<DataFanOut>(dataObject);

        if (dataFanOut)
        {
            double rps = dataFanOut->data().rps;
            rps += (random() % 3) - 1;
            dataFanOut->data().rps = rps;
            dataFanOut->data().rpm = rps * 60;

            postEvent(dataFanOut.get(), CommandEvents::EventValueUpdated);
            continue;
        }

        std::shared_ptr<DataPowerMeter> dataPowerMeter = std::dynamic_pointer_cast<DataPowerMeter>(dataObject);

        if (dataPowerMeter)
        {
            int value = dataPowerMeter->getCurrent_mA();
            value += (random() % 6) - 2;
            if (value < 0) value = 0;
            dataPowerMeter->setCurrent_mA(value);

            value = dataPowerMeter->getLoad_mV();
            value += (random() % 6) - 2;
            if (value < 0) value = 0;
            dataPowerMeter->setLoad_mV(value);

            value = dataPowerMeter->getPower_mW();
            value += (random() % 6) - 2;
            if (value < 0) value = 0;
            dataPowerMeter->setPower_mW(value);

            postEvent(dataPowerMeter.get(), CommandEvents::EventValueUpdated);
            continue;
        }
    }
}
