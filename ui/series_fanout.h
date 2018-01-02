#pragma once

#include "series_data.h"
#include "timeseries_data.h"
#include <QtCharts/QChartGlobal>
#include <QtCore/QContiguousCache>
#include <memory>

class DataFanOut;

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

class SeriesFanOut : public SeriesWithData
{
public:
	SeriesFanOut(std::shared_ptr<DataFanOut> dataFanOut);
    virtual ~SeriesFanOut();

    virtual void updateValues() override;

    virtual void addYAxis(QT_CHARTS_NAMESPACE::QChart *chart) override;
    virtual void addSeries(QT_CHARTS_NAMESPACE::QChart *chart) override;

    virtual void rescaleYAxis() override;
    virtual void setVisible(bool visible) override;

    void setVisibleRpm(bool visible);
    void setVisibleDuty(bool visible);

private:
    std::shared_ptr<DataFanOut> _dataFanOut;

    QContiguousCache<TimeSeriesData> _timeDataSeriesRpm;
    QContiguousCache<TimeSeriesData> _timeDataSeriesDuty;

    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_duty = 0;
	QT_CHARTS_NAMESPACE::QValueAxis *_axisY_rpm = 0;

    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanRpm = 0;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanDuty = 0;
};
