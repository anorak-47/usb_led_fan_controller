#pragma once

#include "timeseries_data.h"
#include <QtCore/QContiguousCache>
#include <QtCharts/QChartGlobal>
#include <QtCore/QObject>

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QChartView;
class QValueAxis;
class QDateTimeAxis;
class QLineSeries;
QT_CHARTS_END_NAMESPACE

class SeriesWithData : public QObject
{
	Q_OBJECT

public:
	SeriesWithData();
    virtual ~SeriesWithData();

    virtual void updateValues() = 0;

    virtual QT_CHARTS_NAMESPACE::QChart *createChart();
    virtual QT_CHARTS_NAMESPACE::QChartView *createChartView(QT_CHARTS_NAMESPACE::QChart *chart = 0);

    virtual void addXAxis(QT_CHARTS_NAMESPACE::QChart *chart);
    virtual void addYAxis(QT_CHARTS_NAMESPACE::QChart *chart);
    virtual void addSeries(QT_CHARTS_NAMESPACE::QChart *chart) = 0;

    virtual void setYAxis(QT_CHARTS_NAMESPACE::QValueAxis *y);
    virtual void setXAxis(QT_CHARTS_NAMESPACE::QDateTimeAxis *x);

    virtual QT_CHARTS_NAMESPACE::QValueAxis* getYAxis() const;
    virtual QT_CHARTS_NAMESPACE::QDateTimeAxis* getXAxis() const;

    virtual void rescaleXAxis(const QDateTime &oldestTimestamp = QDateTime::currentDateTime());
    virtual void rescaleYAxis();

    virtual void setVisible(bool visible) = 0;

    int getXaxisDisplayRange() const;
    void setXaxisDisplayRange(int xaxisDisplayRange);

public slots:
    void on_valueUpdated();

protected:
    void appendLastValueToSeries(QT_CHARTS_NAMESPACE::QLineSeries *series, QContiguousCache<TimeSeriesData> const &timeSeriesData);

    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisX = 0;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY = 0;
    int _xaxis_display_range = 60*60;
};
