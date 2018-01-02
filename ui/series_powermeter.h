#pragma once

#include "series_data.h"
#include "timeseries_data.h"
#include <QtCharts/QChartGlobal>
#include <QtCore/QContiguousCache>
#include <memory>

class DataPowerMeter;

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

class SeriesPowerMeter : public SeriesWithData
{
public:
	SeriesPowerMeter(std::shared_ptr<DataPowerMeter> dataFan);
    virtual ~SeriesPowerMeter();

    virtual void updateValues() override;

    virtual void addYAxis(QT_CHARTS_NAMESPACE::QChart *chart) override;
    virtual void addSeries(QT_CHARTS_NAMESPACE::QChart *chart) override;

    virtual void rescaleYAxis() override;

    virtual void setVisible(bool visible) override;

    void setVisiblePower(bool visible);
    void setVisibleCurrent(bool visible);
    void setVisibleLoad(bool visible);

private:
    std::shared_ptr<DataPowerMeter> _dataPowerMeter;

    QContiguousCache<TimeSeriesData> _timeDataSeriesPower;
    QContiguousCache<TimeSeriesData> _timeDataSeriesCurrent;
    QContiguousCache<TimeSeriesData> _timeDataSeriesLoad;

    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_mA = 0;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_mW = 0;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_V = 0;

    QT_CHARTS_NAMESPACE::QLineSeries *_seriesPower = 0;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesCurrent = 0;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesLoad = 0;
};
