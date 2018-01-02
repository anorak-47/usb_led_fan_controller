#pragma once

#include "data.h"
#include "sensor_type.h"
#include "types.h"
#include "timeseries_data.h"
#include <QtCore/QContiguousCache>
#include <QtCore/QObject>

class QMutex;

class DataSensor : public DataWithAChannel
{
	Q_OBJECT

public:
    DataSensor(int channel);
    virtual ~DataSensor();

    virtual QString name() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    // temperature sensor type, e.g SnsType_Temp_NTC1.
    void updateType(SNSTYPE type);
    void updateValue(unsigned int value);

    void setValue(double value);

    Sensor &data();
    QContiguousCache<TimeSeriesData> const &timeDataSeries() const;

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    Sensor _sensor;
    QContiguousCache<TimeSeriesData> _timeDataSeries;
};
