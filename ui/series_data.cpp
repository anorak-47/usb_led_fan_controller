#include "series_data.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

SeriesWithData::SeriesWithData()
{
}

SeriesWithData::~SeriesWithData()
{
}

void SeriesWithData::scaleXAxis()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime then = now.addSecs(-_xaxis_display_range);
    _axisX->setRange(then, now);
}

void SeriesWithData::rescaleXAxis(QDateTime const &oldestTimestamp)
{
    if (_axisX == 0)
		return;

    QDateTime now = QDateTime::currentDateTime();
    QDateTime then = now.addSecs(-_xaxis_display_range);

    if (oldestTimestamp > then)
    {
        then = oldestTimestamp;
        now = then.addSecs(_xaxis_display_range);
    }

    if (then != _axisX->min() || now != _axisX->max())
        _axisX->setRange(then, now);
}

void SeriesWithData::rescaleYAxis()
{
}

void SeriesWithData::setVisible(bool visible)
{
    _is_visible = visible;
}

bool SeriesWithData::isVisible() const
{
    return _is_visible;
}

void SeriesWithData::setXAxis(QDateTimeAxis *x)
{
    _axisX = x;
}

void SeriesWithData::setYAxis(QValueAxis *y)
{
    _axisY = y;
}

QValueAxis *SeriesWithData::getYAxis() const
{
    return _axisY;
}

QDateTimeAxis *SeriesWithData::getXAxis() const
{
    return _axisX;
}

void SeriesWithData::on_valueUpdated()
{
	 updateValues();
}

int SeriesWithData::getXaxisDisplayRange() const
{
    return _xaxis_display_range;
}

void SeriesWithData::setXaxisDisplayRange(int xaxisDisplayRange)
{
    _xaxis_display_range = xaxisDisplayRange;
}

void SeriesWithData::addXAxis(QChart *chart)
{
    _axisX = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisX->setFormat("h:mm");
    //axisX_fan->setTitleText("Time");

    chart->setAxisX(_axisX);
}

void SeriesWithData::addYAxis(QChart *chart)
{
    _axisY = new QValueAxis();
    _axisY->setLabelFormat("%d");
    _axisY->setTitleText("Value");

    chart->addAxis(_axisY, Qt::AlignLeft);
}

void SeriesWithData::appendLastValueToSeries(QLineSeries *series, QContiguousCache<TimeSeriesData> const &timeSeriesData)
{
    series->append(timeSeriesData.last().dt.toMSecsSinceEpoch(), timeSeriesData.last().value);
}

void SeriesWithData::copyFromTimeSeries(QLineSeries *lineseries, QContiguousCache<TimeSeriesData> const &timeSeries)
{
    lineseries->clear();
    //qDebug() << "copyFromTimeSeries: timeSeries size " << timeSeries.size();
    for (int i = 0; i < timeSeries.size(); i++)
    {
        TimeSeriesData const &point = timeSeries.at(i);
        lineseries->append(point.dt.toMSecsSinceEpoch(), point.value);
    }
}

QChart *SeriesWithData::createChart()
{
	QChart *chart = new QChart();
    //fanChart->legend()->hide();
    //_chartFan->setTitle(_dataFan->fullName());
	chart->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);
	return chart;
}

QChartView *SeriesWithData::createChartView(QChart *chart)
{
	if (chart == 0)
		chart = createChart();

	QChartView *chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);

	addXAxis(chart);
	addYAxis(chart);
	addSeries(chart);

    scaleXAxis();
	rescaleYAxis();

	return chartView;
}
