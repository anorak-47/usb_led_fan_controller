#pragma once

#include "data.h"
#include "types.h"
#include <QtGui/QColor>
#include <QtCore/QObject>

class QMutex;

class DataFastLed : public DataWithAChannel
{
    Q_OBJECT

public:
    DataFastLed(int channel);
    virtual ~DataFastLed();

    virtual QString name() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    void updateAnimationId(unsigned char animationId);
    void updateAutoStart(bool autoStart);
    void updateFps(unsigned char fps);
    void updateRunning(bool running);
    void updateSensorIndex(unsigned char sensorIndex);
    void updateColors(QColor const &color1, QColor const &color2);

    unsigned char getAnimationId() const;
    void setAnimationId(unsigned char animationId);

    bool isAutoStart() const;
    void setAutoStart(bool autoStart);

    unsigned char getFps() const;
    void setFps(unsigned char fps);

    bool isRunning() const;
    void setRunning(bool running);

    unsigned char getSensorIndex() const;
    void setSensorIndex(unsigned char sensorIndex);

    const QColor &getColor1() const;
    void setColor1(const QColor &color1);

    const QColor &getColor2() const;
    void setColor2(const QColor &color2);

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    bool _running = false;
    unsigned char _animationId = 0;
    unsigned char _fps = 1;
    unsigned char _sensorIndex = 0;
    bool _autoStart = false;
    QColor _color1;
    QColor _color2;
};
