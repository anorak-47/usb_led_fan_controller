#pragma once

#include "data.h"
#include "types.h"
#include <QtGui/QColor>
#include <QtCore/QObject>

class QMutex;

class DataPowerMeter : public DataWithAChannel
{
    Q_OBJECT

public:
    DataPowerMeter(int channel);
    virtual ~DataPowerMeter();

    virtual QString name() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    unsigned int getCurrent_mA() const;
    unsigned int getLoad_mV() const;
    unsigned int getPower_mW() const;

    void setCurrent_mA(unsigned int current);
    void setLoad_mV(unsigned int load);
    void setPower_mW(unsigned int power);

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    unsigned int _power_mW;
    unsigned int _current_mA;
    unsigned int _load_mV;
};
