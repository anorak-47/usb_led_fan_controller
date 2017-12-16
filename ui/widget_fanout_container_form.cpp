#include "widget_fanout_container_form.h"
#include "ui_widget_fanout_container_form.h"
#include "widget_fanout_form.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDateTime>

QT_CHARTS_USE_NAMESPACE

WidgetFanoutContainerForm::WidgetFanoutContainerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFanoutContainerForm)
{
    ui->setupUi(this);

    _fanoutLayout = new QVBoxLayout();
    ui->gbFanout->setLayout(_fanoutLayout);

    createChart();
}

WidgetFanoutContainerForm::~WidgetFanoutContainerForm()
{
    delete ui;
}

void WidgetFanoutContainerForm::addWidgetFanout(WidgetFanOutForm *fanout)
{
    _fanoutLayout->addWidget(fanout);
}

void WidgetFanoutContainerForm::createChart()
{
    QValueAxis *axisY_duty = new QValueAxis();
    axisY_duty->setLabelFormat("%d");
    axisY_duty->setTitleText("Duty");

    QValueAxis *axisY_rpm = new QValueAxis();
    axisY_rpm->setLabelFormat("%d");
    axisY_rpm->setTitleText("RPM");

    _axisXFan = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisXFan->setFormat("h:mm");
    //axisX_fan->setTitleText("Time");

    QDateTime now = QDateTime::currentDateTime();
    _axisXFan->setRange(now.addSecs(-60*60), now);

    _chartFan = new QChart();
    //fanChart->legend()->hide();
    _chartFan->setTitle(tr("Fan Output"));
    _chartFan->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chartFan->addAxis(axisY_duty, Qt::AlignLeft);
    _chartFan->addAxis(axisY_rpm, Qt::AlignRight);

    _chartFan->setAxisX(_axisXFan);

    QChartView *chartView = new QChartView(_chartFan);
    chartView->setRenderHint(QPainter::Antialiasing);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wGraph->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _seriesFanRpm = new QLineSeries();
    _seriesFanRpm->setName("RPM");
    _chartFan->addSeries(_seriesFanRpm);
    _seriesFanRpm->attachAxis(_axisXFan);
    _seriesFanRpm->attachAxis(axisY_rpm);

    _seriesFanDuty = new QLineSeries();
    _seriesFanDuty->setName("Duty");
    _chartFan->addSeries(_seriesFanDuty);
    _seriesFanDuty->attachAxis(_axisXFan);
    _seriesFanDuty->attachAxis(axisY_duty);

    axisY_duty->setRange(0.0, 100.0);
    axisY_rpm->setRange(0.0, 5000.0);
}
