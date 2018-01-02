#include "value_updater.h"
#include "data.h"
#include <QtCore/QTimer>
#include <QtCore/QDebug>

ValueUpdater::ValueUpdater()
{
	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
}

ValueUpdater::~ValueUpdater()
{
	delete _timer;
}

void ValueUpdater::registerDataObject(std::shared_ptr<DataObject> dataObject)
{
    dataObjects.push_back(dataObject);
}

void ValueUpdater::startUpdates()
{
    _timer->start(_update_interval_msecs);
}

void ValueUpdater::stopUpdates()
{
	_timer->stop();
}

int ValueUpdater::getUpdateIntervalMsecs() const
{
    return _update_interval_msecs;
}

void ValueUpdater::setUpdateIntervalMsecs(int updateIntervalMsecs = 1000)
{
    _update_interval_msecs = updateIntervalMsecs;
    if (_is_connected)
    	_timer->start(_update_interval_msecs);
}

void ValueUpdater::on_usbConectionChanged(bool connected)
{
	_is_connected = connected;

    if (connected)
    {
        startUpdates();
    }
    else
    {
        stopUpdates();
    }
}

void ValueUpdater::on_timeout()
{
    for (auto &dataObject : dataObjects)
        dataObject->updateValues();
}
