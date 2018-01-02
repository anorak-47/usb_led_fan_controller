#include "connection_watcher.h"
#include "data.h"
#include "data_properties.h"
#include <QtCore/QDebug>

ConnectionWatcher::ConnectionWatcher()
{
}

ConnectionWatcher::~ConnectionWatcher()
{
}

void ConnectionWatcher::registerDataObject(std::shared_ptr<DataObject> dataObject)
{
    _dataObjects.push_back(dataObject);
}

void ConnectionWatcher::registerPropertyDataObject(std::shared_ptr<DataDeviceProperties> dataObject)
{
    _propertyDataObject = dataObject;
    connect(_propertyDataObject.get(), SIGNAL(signalPropertiesUpdated()), this, SLOT(on_popertiesUpdated()));
}

void ConnectionWatcher::on_popertiesUpdated()
{
    if (_connected)
        updateValues();
}

void ConnectionWatcher::updateValues()
{
    for (auto &dataObject : _dataObjects)
        dataObject->update();
}

void ConnectionWatcher::on_usbConectionChanged(bool connected)
{
    _connected = connected;
    if (_propertyDataObject && connected)
        _propertyDataObject->update();
}

void ConnectionWatcher::on_requestResync()
{
    if (_propertyDataObject && _connected)
        _propertyDataObject->update();
}
