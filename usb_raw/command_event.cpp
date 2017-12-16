#include "command_event.h"

CommandEvent::CommandEvent(CommandEvents event, int channel) : QEvent((QEvent::Type)event), _channel(channel)
{
}

int CommandEvent::getChannel() const
{
    return _channel;
}

CommandEventUsbError::CommandEventUsbError(int errorCode, QString const &msg, int channel)
    : CommandEvent(CommandEvents::EventUsbCommunicationError, channel), _errorCode(errorCode), _msg(msg)
{
}

int CommandEventUsbError::errorCode() const
{
    return _errorCode;
}

QString CommandEventUsbError::errorMsg() const
{
    return _msg;
}
