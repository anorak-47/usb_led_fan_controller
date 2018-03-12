#include "series_powermeter.h"
#include "data_powermeter.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

SeriesPowerMeter::SeriesPowerMeter(std::shared_ptr<DataPowerMeter> dataPowerMeter) : _dataPowerMeter(dataPowerMeter)
{
    _timeDataSeriesPower.setCapacity(TIME_SERIES_CAPACITY);
    _timeDataSeriesCurrent.setCapacity(TIME_SERIES_CAPACITY);
    _timeDataSeriesLoad.setCapacity(TIME_SERIES_CAPACITY);

    _seriesPower = new QLineSeries();
    _seriesPower->setName("Power [mW]");

    _seriesCurrent = new QLineSeries();
    _seriesCurrent->setName("Current [mA]");

    _seriesLoad = new QLineSeries();
    _seriesLoad->setName("Load [V]");

    connect(_dataPowerMeter.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));
}

SeriesPowerMeter::~SeriesPowerMeter()
{
    delete _seriesCurrent;
    delete _seriesLoad;
    delete _seriesPower;
}

void SeriesPowerMeter::updateValues()
{
    _timeDataSeriesCurrent.append(TimeSeriesData(_dataPowerMeter->getCurrent_mA()));
    _timeDataSeriesPower.append(TimeSeriesData(_dataPowerMeter->getPower_mW()));
    _timeDataSeriesLoad.append(TimeSeriesData(_dataPowerMeter->getLoad_mV() / 1000));

    if (isVisible())
    {
        qDebug() << "SeriesPowerMeter:" << _dataPowerMeter->fullName() << " update graph";

        appendLastValueToSeries(_seriesPower, _timeDataSeriesPower);
        appendLastValueToSeries(_seriesCurrent, _timeDataSeriesCurrent);
        appendLastValueToSeries(_seriesLoad, _timeDataSeriesLoad);

        rescaleXAxis(_timeDataSeriesPower.first().dt);
    }
}

void SeriesPowerMeter::addYAxis(QChart *chart)
{
    _axisY_mA = new QValueAxis();
    _axisY_mA->setLabelFormat("%d");
    _axisY_mA->setTitleText("Current [mA]");

    _axisY_mW = new QValueAxis();
    _axisY_mW->setLabelFormat("%d");
    _axisY_mW->setTitleText("Power [mW]");

    _axisY_V = new QValueAxis();
    _axisY_V->setLabelFormat("%d");
    _axisY_V->setTitleText("Load [V]");

    chart->addAxis(_axisY_mA, Qt::AlignLeft);
    chart->addAxis(_axisY_mW, Qt::AlignLeft);
    chart->addAxis(_axisY_V, Qt::AlignRight);
}

void SeriesPowerMeter::addSeries(QChart *chart)
{
    chart->addSeries(_seriesPower);
    _seriesPower->attachAxis(_axisX);
    _seriesPower->attachAxis(_axisY_mW);

    chart->addSeries(_seriesCurrent);
    _seriesCurrent->attachAxis(_axisX);
    _seriesCurrent->attachAxis(_axisY_mA);

    chart->addSeries(_seriesLoad);
    _seriesLoad->attachAxis(_axisX);
    _seriesLoad->attachAxis(_axisY_V);
}

void SeriesPowerMeter::rescaleYAxis()
{
    _axisY_mA->setRange(0.0, 1000.0);
    _axisY_mW->setRange(0.0, 2500.0);
    _axisY_V->setRange(0.0, 14.0);
}

void SeriesPowerMeter::setVisible(bool visible)
{
    //qDebug() << "SeriesPowerMeter:" << _dataPowerMeter->fullName() << " visible: " << visible;

    SeriesWithData::setVisible(visible);

    if (visible)
    {
        {
            QSignalBlocker b1(_seriesPower);
            QSignalBlocker b2(_seriesCurrent);
            QSignalBlocker b3(_seriesLoad);

            copyFromTimeSeries(_seriesLoad, _timeDataSeriesLoad);
            copyFromTimeSeries(_seriesCurrent, _timeDataSeriesCurrent);
            copyFromTimeSeries(_seriesPower, _timeDataSeriesPower);
        }
        rescaleXAxis(_timeDataSeriesLoad.first().dt);
    }
}

void SeriesPowerMeter::setVisiblePower(bool visible)
{
    if (_seriesPower->isVisible() != visible)
        _seriesPower->setVisible(visible);
}

void SeriesPowerMeter::setVisibleCurrent(bool visible)
{
    if (_seriesCurrent->isVisible() != visible)
        _seriesCurrent->setVisible(visible);
}

void SeriesPowerMeter::setVisibleLoad(bool visible)
{
    if (_seriesLoad->isVisible() != visible)
        _seriesLoad->setVisible(visible);
}
