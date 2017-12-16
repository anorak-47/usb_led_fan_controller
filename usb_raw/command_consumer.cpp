#include "command_consumer.h"
#include "command_queue.h"
#include "usb_connection.h"
#include <QtCore/QThreadPool>
#include <QtCore/QRunnable>

CommandConsumer::CommandConsumer(QObject *parent) : QThread(parent)
{
}

UsbCommandConsumer::UsbCommandConsumer(std::shared_ptr<ConnectionObject> connectionObject, QObject *parent)
    : CommandConsumer(parent), _connectionObject(connectionObject)
{
}

void UsbCommandConsumer::run()
{
    forever
    {
        std::unique_ptr<Command> cmd = std::move(CommandQueueInstance().dequeue());

        if (isInterruptionRequested())
            return;

        cmd->exec(_connectionObject);
    }
}

class CommandWorker : public QRunnable
{
public:
    CommandWorker(std::shared_ptr<ConnectionObject> connectionObject, std::unique_ptr<Command> cmd)
        : _connectionObject(connectionObject), _cmd(std::move(cmd))
    {
    }

private:
    void run() override
    {
        _cmd->exec(_connectionObject);
    }

    std::shared_ptr<ConnectionObject> _connectionObject;
    std::unique_ptr<Command> _cmd;
};

UsbCommandWorkerConsumer::UsbCommandWorkerConsumer(std::shared_ptr<ConnectionObject> connectionObject, QObject *parent)
    : CommandConsumer(parent), _connectionObject(connectionObject)
{
}

void UsbCommandWorkerConsumer::run()
{
    forever
    {
        std::unique_ptr<Command> cmd = std::move(CommandQueueInstance().dequeue());

        if (isInterruptionRequested())
            return;

        CommandWorker *worker = new CommandWorker(_connectionObject, std::move(cmd));
        while (!QThreadPool::globalInstance()->tryStart(worker))
        {
            QThread::msleep(100);
        }
    }

    QThreadPool::globalInstance()->waitForDone(10000);
}
