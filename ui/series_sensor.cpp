#include "series_sensor.h"
#include "data_sensor.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

SeriesSensor::SeriesSensor(std::shared_ptr<DataSensor> dataSensor) : _dataSensor(dataSensor)
{
    _seriesSensor = new QLineSeries();
    _seriesSensor->setName(_dataSensor->name());

    connect(_dataSensor.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));
}

SeriesSensor::~SeriesSensor()
{
    delete _seriesSensor;
}

void SeriesSensor::updateValues()
{
    //qDebug() << __PRETTY_FUNCTION__ << " timeSeries size " << _dataSensor->timeDataSeries().size();
    if (_dataSensor->timeDataSeries().isEmpty() || !isVisible())
        return;

    appendLastValueToSeries(_seriesSensor, _dataSensor->timeDataSeries());
    rescaleXAxis(_dataSensor->timeDataSeries().first().dt);
}

void SeriesSensor::copyFromTimeSeries(QContiguousCache<TimeSeriesData> const &timeSeries)
{
    SeriesWithData::copyFromTimeSeries(_seriesSensor, timeSeries);
}

void SeriesSensor::addYAxis(QChart *chart)
{
    _axisY = new QValueAxis();
    _axisY->setLabelFormat("%d");
    _axisY->setTitleText("Value");

    chart->addAxis(_axisY, Qt::AlignLeft);
}

void SeriesSensor::addSeries(QChart *chart)
{
    chart->addSeries(_seriesSensor);
    _seriesSensor->attachAxis(_axisX);
    _seriesSensor->attachAxis(_axisY);
}

void SeriesSensor::rescaleYAxis()
{
    _axisY->setRange(0.0, 75.0);
}

void SeriesSensor::setVisible(bool visible)
{
    //qDebug() << "SeriesSensor:" << _dataSensor->fullName() << " visible: " << visible;

    SeriesWithData::setVisible(visible);

    if (visible)
    {
        {
            QSignalBlocker b(_seriesSensor);
            copyFromTimeSeries(_dataSensor->timeDataSeries());
        }
        if (!_dataSensor->timeDataSeries().isEmpty())
            rescaleXAxis(_dataSensor->timeDataSeries().first().dt);
    }
}

void SeriesSensor::setSeriesVisible(bool visible)
{
    if (_seriesSensor->isVisible() != visible)
        _seriesSensor->setVisible(visible);    
}

void SeriesSensor::setName(const QString &name)
{
    _seriesSensor->setName(name);
}

void SeriesSensor::setColor(QColor color)
{
    if (_seriesSensor->color() != color)
        _seriesSensor->setColor(color);
}
