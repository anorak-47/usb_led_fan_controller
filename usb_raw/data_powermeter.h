#pragma once

#include "data.h"
#include "types.h"
#include "timeseries_data.h"
#include <QtGui/QColor>
#include <QtCore/QObject>
#include <QtCore/QContiguousCache>

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

    QContiguousCache<TimeSeriesData> seriesPower();
    QContiguousCache<TimeSeriesData> seriesCurrent();
    QContiguousCache<TimeSeriesData> seriesLoad();

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    unsigned int _power_mW;
    unsigned int _current_mA;
    unsigned int _load_mV;

    QContiguousCache<TimeSeriesData> _seriesPower;
    QContiguousCache<TimeSeriesData> _seriesCurrent;
    QContiguousCache<TimeSeriesData> _seriesLoad;
};
