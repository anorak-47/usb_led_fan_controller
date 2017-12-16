#pragma once
#include <QtCore/QObject>
#include <memory>
#include <list>

class DataObject;
class QTimer;

class ValueUpdater : public QObject
{
    Q_OBJECT

public:
    ValueUpdater();
    virtual ~ValueUpdater();

    int getUpdateIntervalMsecs() const;
    void setUpdateIntervalMsecs(int updateIntervalMsecs);

    void registerDataObject(std::shared_ptr<DataObject> dataObject);

    void stopUpdates();
    void startUpdates();

public slots:
    void on_usbConectionChanged(bool connected);

private slots:
    void on_timeout();

private:
    std::list<std::shared_ptr<DataObject>> dataObjects;
    QTimer *_timer;
    int _update_interval_msecs = 5000;
    bool _is_connected = false;
};
