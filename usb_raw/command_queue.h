#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include "command.h"
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <memory>
#include <queue>


class CommandQueue
{
public:
    CommandQueue();

    void enqueue(std::unique_ptr<Command> cmd);
    std::unique_ptr<Command> dequeue();
    bool empty();

    void requestInterruption();

private:
    bool _interruptionRequested;
    QWaitCondition bufferNotEmpty;
    QWaitCondition bufferNotFull;
    QMutex mutex;

    std::queue<std::unique_ptr<Command>> _queue;
};

class CommandQueueInstance
{
public:
    CommandQueueInstance();

    void enqueue(std::unique_ptr<Command> cmd);
    std::unique_ptr<Command> dequeue();
    bool empty();

    void requestInterruption();

private:
    static std::unique_ptr<CommandQueue> _queue;
};

#endif // CommandQueue_H
