#pragma once
#include <QtCore/QObject>
#include <memory>
#include <list>

class DataObject;

class ConnectionWatcher: public QObject
{
	Q_OBJECT

public:
    ConnectionWatcher();
    virtual ~ConnectionWatcher();

    void registerDataObject(std::shared_ptr<DataObject> dataObject);

public slots:
	void on_usbConectionChanged(bool connected);
    void on_requestResync();

protected:
    void updateValues();

private:
    std::list<std::shared_ptr<DataObject>> dataObjects;
};
