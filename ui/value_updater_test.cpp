#include "value_updater_test.h"
#include "data.h"
#include "data_sensor.h"
#include "data_fan.h"
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
    }
}
