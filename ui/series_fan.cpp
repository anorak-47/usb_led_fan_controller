#include "series_fan.h"
#include "data_fan.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

SeriesFan::SeriesFan(std::shared_ptr<DataFan> dataFan)
	: _dataFan(dataFan)
{
    _timeDataSeriesRpm.setCapacity(TIME_SERIES_CAPACITY);
    _timeDataSeriesDuty.setCapacity(TIME_SERIES_CAPACITY);
    _timeDataSeriesSetpoint.setCapacity(TIME_SERIES_CAPACITY);

    QLineSeries *series = new QLineSeries();
    series->setName("RPM [1/s]");
    _series[SeriesType::rpm] = series;

    series = new QLineSeries();
    series->setName("Duty Cycle [%]");
    _series[SeriesType::dutycycle] = series;

    series = new QLineSeries();
    series->setName("PI Setpoint");
    _series[SeriesType::setpoint] = series;

    connect(_dataFan.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));
}

SeriesFan::~SeriesFan()
{
    //for (auto series : _series)
    //    delete series->second;
}

void SeriesFan::updateValues()
{
    _timeDataSeriesDuty.append(TimeSeriesData(_dataFan->getScaledDuty()));
    _timeDataSeriesRpm.append(TimeSeriesData(_dataFan->getRPM()));
    _timeDataSeriesSetpoint.append(TimeSeriesData(_dataFan->getSetpointValue()));

    if (SeriesWithData::isVisible())
    {
        qDebug() << "SeriesFan:" << _dataFan->fullName() << " update graph";

        appendLastValueToSeries(_series[SeriesType::rpm], _timeDataSeriesRpm);
        appendLastValueToSeries(_series[SeriesType::dutycycle], _timeDataSeriesDuty);
        appendLastValueToSeries(_series[SeriesType::setpoint], _timeDataSeriesSetpoint);

        rescaleXAxis(_timeDataSeriesRpm.first().dt);
        rescaleYAxis();
    }
}

void SeriesFan::addYAxis(QChart *chart)
{
	_axisY = new QValueAxis();
	_axisY->setLabelFormat("%d");
    _axisY->setTitleText("Sensor Value");

	_axisY_duty = new QValueAxis();
	_axisY_duty->setLabelFormat("%d");
    _axisY_duty->setTitleText("Fan Duty Cycle [%]");

	_axisY_rpm = new QValueAxis();
	_axisY_rpm->setLabelFormat("%d");
    _axisY_rpm->setTitleText("Fan RPM [1/s]");

    chart->addAxis(_axisY, Qt::AlignRight);
	chart->addAxis(_axisY_duty, Qt::AlignLeft);
    chart->addAxis(_axisY_rpm, Qt::AlignLeft);
}

void SeriesFan::addSeries(QChart *chart)
{
    chart->addSeries(_series[SeriesType::rpm]);
    _series[SeriesType::rpm]->attachAxis(_axisX);
    _series[SeriesType::rpm]->attachAxis(_axisY_rpm);

    chart->addSeries(_series[SeriesType::dutycycle]);
    _series[SeriesType::dutycycle]->attachAxis(_axisX);
    _series[SeriesType::dutycycle]->attachAxis(_axisY_duty);

    chart->addSeries(_series[SeriesType::setpoint]);
    _series[SeriesType::setpoint]->attachAxis(_axisX);
    _series[SeriesType::setpoint]->attachAxis(_axisY);
}

void SeriesFan::rescaleYAxis()
{
    _axisY->setRange(0.0, 50.0);
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
}

void SeriesFan::setVisible(bool visible)
{
    qDebug() << "SeriesFan:" << _dataFan->fullName() << " visible: " << visible;

    SeriesWithData::setVisible(visible);

    if (visible)
    {
        {
            QSignalBlocker b1(_series[SeriesType::rpm]);
            QSignalBlocker b2(_series[SeriesType::dutycycle]);
            QSignalBlocker b3(_series[SeriesType::setpoint]);

            copyFromTimeSeries(_series[SeriesType::rpm], _timeDataSeriesRpm);
            copyFromTimeSeries(_series[SeriesType::dutycycle], _timeDataSeriesDuty);
            copyFromTimeSeries(_series[SeriesType::setpoint], _timeDataSeriesSetpoint);
        }
        rescaleXAxis(_timeDataSeriesRpm.first().dt);
    }
}

void SeriesFan::setVisible(SeriesType type, bool visible)
{
    if (_series[type]->isVisible() != visible)
        _series[type]->setVisible(visible);
}

void SeriesFan::setColor(SeriesFan::SeriesType type, QColor color)
{
    if (_series[type]->color() != color)
        _series[type]->setColor(color);
}

void SeriesFan::setName(SeriesFan::SeriesType type, const QString &name)
{
    if (_series[type]->name() != name)
        _series[type]->setName(name);
}

void SeriesFan::setYAxis(QValueAxis *y, QValueAxis *y_duty, QValueAxis *y_rpm)
{
    _axisY = y;
    _axisY_duty = y_duty;
    _axisY_rpm = y_rpm;
}
