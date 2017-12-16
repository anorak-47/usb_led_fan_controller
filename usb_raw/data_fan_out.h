#pragma once

#include "data.h"
#include "fan_out_type.h"
#include "types.h"
#include "timeseries_data.h"
#include <QtCore/QObject>
#include <QtCore/QContiguousCache>

class QMutex;

class DataFanOut : public DataWithAChannel
{
	Q_OBJECT

public:
    DataFanOut(int channel);
    virtual ~DataFanOut();

    virtual QString name() const override;
    virtual QString description() const override;

    virtual void update() override;
    virtual void updateValues() override;

    void updateMode(FANOUTMODE mode);

    Fan_out &data();

protected:
    virtual bool handleEvent(CommandEvent *event) override;

private:
    Fan_out _fanout;

    QContiguousCache<TimeSeriesData> _series;
};
