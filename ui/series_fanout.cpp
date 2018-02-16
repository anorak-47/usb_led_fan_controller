#include "series_fanout.h"
#include "data_fan_out.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

SeriesFanOut::SeriesFanOut(std::shared_ptr<DataFanOut> dataFanOut)
	: _dataFanOut(dataFanOut)
{
    _timeDataSeriesRpm.setCapacity(TIME_SERIES_CAPACITY);
    _timeDataSeriesDuty.setCapacity(TIME_SERIES_CAPACITY);

    _seriesFanRpm = new QLineSeries();
    _seriesFanRpm->setName("Fan RPM");

    _seriesFanDuty = new QLineSeries();
    _seriesFanDuty->setName("Fan Duty Cycle");

    connect(_dataFanOut.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));
}

SeriesFanOut::~SeriesFanOut()
{
    delete _seriesFanDuty;
    delete _seriesFanRpm;
}

void SeriesFanOut::updateValues()
{
	//TODO: use sensor value
    _timeDataSeriesDuty.append(TimeSeriesData(_dataFanOut->data().rpm));
    _timeDataSeriesRpm.append(TimeSeriesData(_dataFanOut->data().rpm));

    if (isVisible())
    {
        qDebug() << "SeriesFanOut:" << _dataFanOut->fullName() << " update graph";
        appendLastValueToSeries(_seriesFanRpm, _timeDataSeriesRpm);
        appendLastValueToSeries(_seriesFanDuty, _timeDataSeriesDuty);

        rescaleXAxis(_timeDataSeriesRpm.first().dt);
    }
}

void SeriesFanOut::addYAxis(QChart *chart)
{
	_axisY_duty = new QValueAxis();
	_axisY_duty->setLabelFormat("%d");
	_axisY_duty->setTitleText("Duty Cycle [%]");

	_axisY_rpm = new QValueAxis();
	_axisY_rpm->setLabelFormat("%d");
	_axisY_rpm->setTitleText("RPM [1/s]");

	chart->addAxis(_axisY_duty, Qt::AlignLeft);
	chart->addAxis(_axisY_rpm, Qt::AlignRight);
}

void SeriesFanOut::addSeries(QChart *chart)
{
    chart->addSeries(_seriesFanRpm);
    _seriesFanRpm->attachAxis(_axisX);
    _seriesFanRpm->attachAxis(_axisY_rpm);

    chart->addSeries(_seriesFanDuty);
    _seriesFanDuty->attachAxis(_axisX);
    _seriesFanDuty->attachAxis(_axisY_duty);
}

void SeriesFanOut::rescaleYAxis()
{
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
}

void SeriesFanOut::setVisible(bool visible)
{
    qDebug() << "SeriesFanOut:" << _dataFanOut->fullName() << " visible: " << visible;

    SeriesWithData::setVisible(visible);

    if (visible)
    {
        {
            QSignalBlocker b1(_seriesFanRpm);
            QSignalBlocker b2(_seriesFanDuty);

            copyFromTimeSeries(_seriesFanRpm, _timeDataSeriesRpm);
            copyFromTimeSeries(_seriesFanDuty, _timeDataSeriesDuty);
        }
        rescaleXAxis(_timeDataSeriesRpm.first().dt);
    }
}

void SeriesFanOut::setVisibleRpm(bool visible)
{
    _seriesFanRpm->setVisible(visible);
}

void SeriesFanOut::setVisibleDuty(bool visible)
{
    _seriesFanDuty->setVisible(visible);
}
