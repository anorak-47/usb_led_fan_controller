#pragma once

#include "series_data.h"
#include "timeseries_data.h"
#include <QtCharts/QChartGlobal>
#include <QtCore/QContiguousCache>
#include <memory>
#include <map>

class DataFan;

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

class SeriesFan : public SeriesWithData
{
public:
    enum class SeriesType
    {
        rpm,
        dutycycle,
        setpoint
    };

	SeriesFan(std::shared_ptr<DataFan> dataFan);
    virtual ~SeriesFan();

    virtual void updateValues() override;    

    virtual void addYAxis(QT_CHARTS_NAMESPACE::QChart *chart) override;
    virtual void addSeries(QT_CHARTS_NAMESPACE::QChart *chart) override;

    virtual void rescaleYAxis() override;

    virtual void setVisible(bool visible) override;

    void setVisible(SeriesType type, bool visible);
    void setColor(SeriesType type, QColor color);
    void setName(SeriesType type, QString const &name);

    void setYAxis(QT_CHARTS_NAMESPACE::QValueAxis *y, QT_CHARTS_NAMESPACE::QValueAxis *y_duty, QT_CHARTS_NAMESPACE::QValueAxis *y_rpm);

private:
    std::shared_ptr<DataFan> _dataFan;

    QContiguousCache<TimeSeriesData> _timeDataSeriesRpm;
    QContiguousCache<TimeSeriesData> _timeDataSeriesDuty;
    QContiguousCache<TimeSeriesData> _timeDataSeriesSetpoint;

    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_duty = 0;
	QT_CHARTS_NAMESPACE::QValueAxis *_axisY_rpm = 0;

    std::map<SeriesType, QT_CHARTS_NAMESPACE::QLineSeries *> _series;
};
