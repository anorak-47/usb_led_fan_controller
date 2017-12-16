#include "connection_watcher.h"
#include "data.h"
#include <QtCore/QDebug>

ConnectionWatcher::ConnectionWatcher()
{
}

ConnectionWatcher::~ConnectionWatcher()
{
}

void ConnectionWatcher::registerDataObject(std::shared_ptr<DataObject> dataObject)
{
    dataObjects.push_back(dataObject);
}

void ConnectionWatcher::updateValues()
{
    for (auto &dataObject : dataObjects)
        dataObject->update();
}

void ConnectionWatcher::on_usbConectionChanged(bool connected)
{
    if (connected)
    {
    	updateValues();
    }
}

void ConnectionWatcher::on_requestResync()
{
	updateValues();
}
