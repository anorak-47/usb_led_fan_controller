#pragma once

#include "series_data.h"
#include "timeseries_data.h"
#include <QtCharts/QChartGlobal>
#include <QtCore/QContiguousCache>
#include <memory>

class DataSensor;

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

class SeriesSensor : public SeriesWithData
{
public:
    SeriesSensor(std::shared_ptr<DataSensor> dataSensor);
    virtual ~SeriesSensor();

    virtual void updateValues() override;

    virtual void addYAxis(QT_CHARTS_NAMESPACE::QChart *chart) override;
    virtual void addSeries(QT_CHARTS_NAMESPACE::QChart *chart) override;

    virtual void rescaleYAxis() override;

    void setVisible(bool visible) override;
    void setName(QString const &name);
    void setColor(QColor color);

    void copyFromTimeSeries(const QContiguousCache<TimeSeriesData> &timeSeries);

private:
    std::shared_ptr<DataSensor> _dataSensor;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesSensor = 0;
};
