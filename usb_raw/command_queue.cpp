#include "command_queue.h"
#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>

#define MAX_COMSUMER_QUEUE_SIZE 100

CommandQueue::CommandQueue() :
    _interruptionRequested(false)
{

}

void CommandQueue::enqueue(std::unique_ptr<Command> cmd)
{
    //qDebug() << "enqueue " << cmd->getName();

    mutex.lock();
    if (_queue.size() > MAX_COMSUMER_QUEUE_SIZE)
        bufferNotFull.wait(&mutex);
    mutex.unlock();

    mutex.lock();
    _queue.push(std::move(cmd));
    //qDebug() << "enqueue " << _queue.size();
    bufferNotEmpty.wakeAll();
    mutex.unlock();
}

std::unique_ptr<Command> CommandQueue::dequeue()
{
    mutex.lock();
    if (_queue.empty())
        bufferNotEmpty.wait(&mutex);

    if (_interruptionRequested)
    {
        qDebug() << "dequeue _interruptionRequested";
        mutex.unlock();
        return std::unique_ptr<Command>();
    }

    std::unique_ptr<Command> cmd = std::move(_queue.front());
    _queue.pop();

    //qDebug() << "dequeue " << cmd->getName();
    //qDebug() << "dequeue " << _queue.size();

    bufferNotFull.wakeAll();
    mutex.unlock();

    return cmd;
}

bool CommandQueue::empty()
{
    QMutexLocker locker(&mutex);
    return _queue.empty();
}

void CommandQueue::requestInterruption()
{
    qDebug() << "requestInterruption";

    QMutexLocker locker(&mutex);
    _interruptionRequested = true;
    bufferNotEmpty.wakeAll();
}

std::unique_ptr<CommandQueue> CommandQueueInstance::_queue(std::unique_ptr<CommandQueue>(new CommandQueue));

CommandQueueInstance::CommandQueueInstance()
{

}

void CommandQueueInstance::enqueue(std::unique_ptr<Command> cmd)
{
    _queue->enqueue(std::move(cmd));
}

std::unique_ptr<Command> CommandQueueInstance::dequeue()
{
    return _queue->dequeue();
}

bool CommandQueueInstance::empty()
{
    return _queue->empty();
}

void CommandQueueInstance::requestInterruption()
{
    _queue->requestInterruption();
}
