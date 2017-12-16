	#include "data_fastled.h"
#include "command.h"
#include "command_queue.h"
#include "command_event.h"
#include <QtCore/QMutex>
#include <QtCore/QDebug>

DataFastLed::DataFastLed(int channel) : DataWithAChannel(channel)
{
}

DataFastLed::~DataFastLed()
{
}

QString DataFastLed::name() const
{
    return QString("LED Stripe %1").arg(_channel + 1);
}

QString DataFastLed::description() const
{
    return "FastLED";
}

void DataFastLed::update()
{
    CommandQueueInstance().enqueue(std::move(std::unique_ptr<CommandUpdateFastLed>(new CommandUpdateFastLed(this))));
}

void DataFastLed::updateValues()
{
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandUpdateFastLedState>(new CommandUpdateFastLedState(this))));
}

void DataFastLed::updateAnimationId(unsigned char animationId)
{
    QMutexLocker l(_mutex);
    _animationId = animationId;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedAnimationId>(new CommandSetFastLedAnimationId(this))));
}

void DataFastLed::updateAutoStart(bool autoStart)
{
    QMutexLocker l(_mutex);
    _autoStart = autoStart;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedConfiguration>(new CommandSetFastLedConfiguration(this))));
}

void DataFastLed::updateFps(unsigned char fps)
{
    QMutexLocker l(_mutex);
    _fps = fps;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedConfiguration>(new CommandSetFastLedConfiguration(this))));
}

void DataFastLed::updateRunning(bool running)
{
    QMutexLocker l(_mutex);
    _running = running;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedState>(new CommandSetFastLedState(this))));
}

void DataFastLed::updateSensorIndex(unsigned char sensorIndex)
{
    QMutexLocker l(_mutex);
    _sensorIndex = sensorIndex;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedConfiguration>(new CommandSetFastLedConfiguration(this))));
}

void DataFastLed::updateColors(QColor const &color1, QColor const &color2)
{
	QMutexLocker l(_mutex);
	_color1 = color1;
	_color2 = color2;
    CommandQueueInstance().enqueue(
        std::move(std::unique_ptr<CommandSetFastLedConfiguration>(new CommandSetFastLedConfiguration(this))));
}

unsigned char DataFastLed::getAnimationId() const
{
	QMutexLocker l(_mutex);
    return _animationId;
}

void DataFastLed::setAnimationId(unsigned char animationId)
{
	QMutexLocker l(_mutex);
    _animationId = animationId;
}

bool DataFastLed::isAutoStart() const
{
	QMutexLocker l(_mutex);
    return _autoStart;
}

void DataFastLed::setAutoStart(bool autoStart)
{
	QMutexLocker l(_mutex);
    _autoStart = autoStart;
}

unsigned char DataFastLed::getFps() const
{
	QMutexLocker l(_mutex);
    return _fps;
}

void DataFastLed::setFps(unsigned char fps)
{
	QMutexLocker l(_mutex);
    _fps = fps;
}

bool DataFastLed::isRunning() const
{
	QMutexLocker l(_mutex);
    return _running;
}

void DataFastLed::setRunning(bool running)
{
	QMutexLocker l(_mutex);
    _running = running;
}

unsigned char DataFastLed::getSensorIndex() const
{
	QMutexLocker l(_mutex);
    return _sensorIndex;
}

void DataFastLed::setSensorIndex(unsigned char sensorIndex)
{
	QMutexLocker l(_mutex);
    _sensorIndex = sensorIndex;
}

const QColor &DataFastLed::getColor1() const
{
	QMutexLocker l(_mutex);
    return _color1;
}

void DataFastLed::setColor1(const QColor &color1)
{
	QMutexLocker l(_mutex);
    _color1 = color1;
}

const QColor &DataFastLed::getColor2() const
{
	QMutexLocker l(_mutex);
    return _color2;
}

void DataFastLed::setColor2(const QColor &color2)
{
	QMutexLocker l(_mutex);
    _color2 = color2;
}

bool DataFastLed::handleEvent(CommandEvent *event)
{
    if (event->type() == (QEvent::Type)CommandEvents::EventValueUpdated)
    {
        qDebug() << " EventValueUpdated event";

        emit signalValueChanged();
        return true;
    }

    return false;
}