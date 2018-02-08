#include "widget_powermeter_form.h"
#include "ui_widget_powermeter_form.h"
#include "data_powermeter.h"
#include "series_powermeter.h"
#include "types.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>

QT_CHARTS_USE_NAMESPACE

WidgetPowerMeterForm::WidgetPowerMeterForm(std::shared_ptr<DataPowerMeter> dataPowerMeter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetPowerMeterForm),
    _dataPowerMeter(dataPowerMeter)
{
    ui->setupUi(this);
    ui->lTitle->setText(dataPowerMeter->fullName());
    _seriesPowerMeter = std::shared_ptr<SeriesPowerMeter>(new SeriesPowerMeter(_dataPowerMeter));

    on_supportedFunctionsUpdated(SUPPORTED_NONE);

    connect(_dataPowerMeter.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataPowerMeter.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));

    createChart();
}

WidgetPowerMeterForm::~WidgetPowerMeterForm()
{
    delete ui;
}

void WidgetPowerMeterForm::on_supportedFunctionsUpdated(int supportedFunctions)
{
    ui->gbPowerMeter->setEnabled((supportedFunctions & SUPPORTED_SNS_POWER));
    _dataPowerMeter->setEnabled((supportedFunctions & SUPPORTED_SNS_POWER));
}

void WidgetPowerMeterForm::appendLastValueToSeries(QLineSeries *series, QContiguousCache<TimeSeriesData> const &timeSeriesData)
{
    series->append(timeSeriesData.last().dt.toMSecsSinceEpoch(), timeSeriesData.last().value);
}

void WidgetPowerMeterForm::on_dataUpdated()
{
    ui->spCurrent->setValue(_dataPowerMeter->getCurrent_mA());
    ui->spLoad->setValue(_dataPowerMeter->getLoad_mV() / 1000.0);
    ui->spPower->setValue(_dataPowerMeter->getPower_mW());
}

void WidgetPowerMeterForm::on_valueUpdated()
{
    on_dataUpdated();
}

void WidgetPowerMeterForm::createChart()
{
    QChartView *chartView = _seriesPowerMeter->createChartView();

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

	/*
    QValueAxis *axisY_mA = new QValueAxis();
    axisY_mA->setLabelFormat("%d");
    axisY_mA->setTitleText("Current [mA]");

    QValueAxis *axisY_mW = new QValueAxis();
    axisY_mW->setLabelFormat("%d");
    axisY_mW->setTitleText("Power [mW]");

    QValueAxis *axisY_V = new QValueAxis();
    axisY_V->setLabelFormat("%d");
    axisY_V->setTitleText("Load [V]");

    _axisX = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisX->setFormat("h:mm");
    //axisX_fan->setTitleText("Time");

    QDateTime now = QDateTime::currentDateTime();
    _axisX->setRange(now.addSecs(-60*60), now);

    _chart = new QChart();
    //_chart->setTitle(tr("Power Meter"));
    _chart->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);

    _chart->addAxis(axisY_mA, Qt::AlignLeft);
    _chart->addAxis(axisY_mW, Qt::AlignLeft);
    _chart->addAxis(axisY_V, Qt::AlignRight);

    _chart->setAxisX(_axisX);

    QChartView *chartView = new QChartView(_chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _seriesPower = new QLineSeries();
    _seriesPower->setName("Power [mW]");
    _chart->addSeries(_seriesPower);
    _seriesPower->attachAxis(_axisX);
    _seriesPower->attachAxis(axisY_mW);

    _seriesCurrent = new QLineSeries();
    _seriesCurrent->setName("Current [mA]");
    _chart->addSeries(_seriesCurrent);
    _seriesCurrent->attachAxis(_axisX);
    _seriesCurrent->attachAxis(axisY_mA);

    _seriesLoad = new QLineSeries();
    _seriesLoad->setName("Load [V]");
    _chart->addSeries(_seriesLoad);
    _seriesLoad->attachAxis(_axisX);
    _seriesLoad->attachAxis(axisY_V);

    axisY_mA->setRange(0.0, 1000.0);
    axisY_mW->setRange(0.0, 5000.0);
    axisY_V->setRange(0.0, 14.0);
    */
}
