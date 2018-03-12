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
    on_supportedFunctionsUpdated(SUPPORTED_SNS_POWER);

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

void WidgetPowerMeterForm::on_currentTabChanged(int index)
{
    Q_UNUSED(index);
    _seriesPowerMeter->setVisible(isVisible());
}

void WidgetPowerMeterForm::createChart()
{
    QChartView *chartView = _seriesPowerMeter->createChartView();

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);
}
