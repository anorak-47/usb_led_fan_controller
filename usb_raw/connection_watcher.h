#pragma once
#include <QtCore/QObject>
#include <memory>
#include <list>

class DataObject;
class DataDeviceProperties;

class ConnectionWatcher: public QObject
{
	Q_OBJECT

public:
    ConnectionWatcher();
    virtual ~ConnectionWatcher();

    void registerDataObject(std::shared_ptr<DataObject> dataObject);
    void registerPropertyDataObject(std::shared_ptr<DataDeviceProperties> dataObject);

public slots:
    void on_popertiesUpdated();
	void on_usbConectionChanged(bool connected);
    void on_requestResync();

protected:
    void updateValues();

private:
    bool _connected = false;
    std::shared_ptr<DataDeviceProperties> _propertyDataObject;
    std::list<std::shared_ptr<DataObject>> _dataObjects;
};
