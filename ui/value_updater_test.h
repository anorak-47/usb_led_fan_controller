#ifndef VALUE_UPDATER_TEST_H
#define VALUE_UPDATER_TEST_H

#include "command_event.h"
#include <QtCore/QObject>
#include <memory>
#include <list>

class QTimer;
class DataObject;
class DataWithAChannel;

class ValueUpdaterTest : public QObject
{
    Q_OBJECT

public:
    ValueUpdaterTest();
    ~ValueUpdaterTest();

    void registerDataObject(std::shared_ptr<DataObject> dataObject);

    void stopUpdates();
    void startUpdates();

private slots:
    void on_timeout();

private:
    void postEvent(DataWithAChannel *caller, CommandEvents eventtype);

    std::list<std::shared_ptr<DataObject>> dataObjects;
    QTimer *_timer;
};

#endif // VALUE_UPDATER_TEST_H
