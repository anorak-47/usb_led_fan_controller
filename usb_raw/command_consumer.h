#ifndef CommandCONSUMER_H
#define CommandCONSUMER_H

#include <QtCore/QThread>
#include <memory>

class ConnectionObject;

class CommandConsumer : public QThread
{
    Q_OBJECT

public:
    CommandConsumer(QObject *parent = NULL);
};

class UsbCommandConsumer : public CommandConsumer
{
public:
    UsbCommandConsumer(std::shared_ptr<ConnectionObject> connectionObject, QObject *parent = NULL);

    void run() override;

protected:
    std::shared_ptr<ConnectionObject> _connectionObject;
};

class UsbCommandWorkerConsumer : public CommandConsumer
{
public:
    UsbCommandWorkerConsumer(std::shared_ptr<ConnectionObject> connectionObject, QObject *parent = NULL);

    void run() override;

protected:
    std::shared_ptr<ConnectionObject> _connectionObject;
};

#endif // CommandCONSUMER_H
