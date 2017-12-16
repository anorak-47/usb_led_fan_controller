#ifndef COMMAND_EVENT_H
#define COMMAND_EVENT_H

#include <QtCore/QEvent>
#include <QtCore/QString>

enum class CommandEvents
{
    EventAllDataUpdated = QEvent::User,
    EventValueUpdated,
	EventPropertiesUpdated,
    EventCommandFinished,
    EventUsbCommunicationError
};

class CommandEvent : public QEvent
{
public:
    CommandEvent(CommandEvents event, int channel = -1);

    int getChannel() const;

private:
    int _channel = -1;
};

class CommandEventUsbError : public CommandEvent
{
public:
    CommandEventUsbError(int errorCode, QString const &msg, int channel = -1);

    int errorCode() const;
    QString errorMsg() const;

private:
    int _errorCode = -1;
    QString _msg;
};

#endif
