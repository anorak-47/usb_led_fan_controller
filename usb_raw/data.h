#pragma once

#include <QtCore/QObject>

class QMutex;
class QTimer;
class CommandEvent;

class DataObject : public QObject
{
	Q_OBJECT

public:
    DataObject();
    virtual ~DataObject();

    virtual QString name() const = 0;
    virtual QString fullName() const;
    virtual QString description() const = 0;

    virtual void update() = 0;
    virtual void updateValues() = 0;

    virtual bool event(QEvent *event) override;

    QMutex *mutex() const;

public slots:
    void on_update();

signals:
    void signalChanged();
    void signalValueChanged();
    void signalFinished();

    void signalUsbCommunicationFailed(int errorCode, QString const& errorMsg);

protected:
	virtual bool handleEvent(CommandEvent *event);

    QMutex *_mutex;
};

class DataWithAChannel : public DataObject
{
	Q_OBJECT

public:
    DataWithAChannel(int channel);
    virtual ~DataWithAChannel();

    int channel() const;

    virtual bool event(QEvent *event) override;

protected:
    int _channel;
};
