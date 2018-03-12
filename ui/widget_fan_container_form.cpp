#include "widget_fan_container_form.h"
#include "ui_widget_fan_container_form.h"
#include "widget_fan_show_form.h"
#include "config.h"
#include "data_fan.h"
#include "series_fan.h"
#include "data_sensor.h"
#include "series_sensor.h"
#include <QLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QSignalMapper>
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

QT_CHARTS_USE_NAMESPACE

WidgetFanContainerForm::WidgetFanContainerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFanContainerForm)
{
    ui->setupUi(this);

    QWidget *w = new QWidget();

    _layout = new QVBoxLayout();
    w->setLayout(_layout);

    ui->scrollArea->setWidget(w);

    _signalMapperValueUpdated = new QSignalMapper(this);
    connect(_signalMapperValueUpdated, SIGNAL(mapped(QWidget*)), this, SLOT(on_sensorValueUpdated(QWidget*)));

    _signalMapperShowGraph = new QSignalMapper(this);
    connect(_signalMapperShowGraph, SIGNAL(mapped(QWidget*)), this, SLOT(on_showGraphUpdated(QWidget*)));

    _signalMapperColorGraph = new QSignalMapper(this);
    connect(_signalMapperColorGraph, SIGNAL(mapped(QWidget*)), this, SLOT(on_colorGraphUpdated(QWidget*)));

    createFanChart();
    readSettings();
}

WidgetFanContainerForm::~WidgetFanContainerForm()
{
    saveSettings();
    delete ui;
}

void WidgetFanContainerForm::on_currentTabChanged(int index)
{
    Q_UNUSED(index);

    bool visible = isVisible();

    for (auto fanShowForm: _widgetFanShowForms)
    {
        fanShowForm->updateHeaderText();

        fanShowForm->seriesFan()->setVisible(visible);
        fanShowForm->seriesSensor()->setVisible(visible);

        fanShowForm->seriesFan()->setVisible(SeriesFan::SeriesType::rpm, fanShowForm->showGraphRpm() && visible);
        fanShowForm->seriesFan()->setVisible(SeriesFan::SeriesType::dutycycle, fanShowForm->showGraphDuty() && visible);
        fanShowForm->seriesSensor()->setSeriesVisible(fanShowForm->showGraphSensor() && visible);
    }
}

void WidgetFanContainerForm::saveSettings()
{
    QSettings settings("Anorak", "ULFControl");
    settings.setValue("FanContainer/splitter", ui->splitter->saveGeometry());

    int c = 0;
    for(int size : ui->splitter->sizes())
    {
        settings.setValue(QString("FanContainer/splitter-size-%1").arg(c++), size);
    }
}

void WidgetFanContainerForm::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    restoreSplitter();
}

void WidgetFanContainerForm::restoreSplitter()
{
    QSettings settings("Anorak", "ULFControl");

    QList <int> sizes;
    for(int c = 0; c < ui->splitter->sizes().size(); c++)
    {
        int restoredSize = settings.value(QString("FanContainer/splitter-size-%1").arg(c), 350).toInt();
        sizes.append(restoredSize);
    }

    ui->splitter->setSizes(sizes);
}

void WidgetFanContainerForm::addWidgetFan(WidgetFanShowForm *fanForm, bool hasSuccessor)
{
    _widgetFanShowForms.push_back(fanForm);
    _layout->addWidget(fanForm);

    if (hasSuccessor)
    {
        QFrame *line = new QFrame(this);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _layout->addWidget(line);
    }

    connect(fanForm, SIGNAL(signalValueUpdated()), _signalMapperValueUpdated, SLOT(map()));
    _signalMapperValueUpdated->setMapping(fanForm, fanForm);

    connect(fanForm, SIGNAL(signalShowGraphChanged()), _signalMapperShowGraph, SLOT(map()));
    _signalMapperShowGraph->setMapping(fanForm, fanForm);

    connect(fanForm, SIGNAL(signalGraphColorChanged()), _signalMapperColorGraph, SLOT(map()));
    _signalMapperColorGraph->setMapping(fanForm, fanForm);

    fanForm->seriesFan()->setXAxis(_axisXFan);
    fanForm->seriesFan()->setYAxis(_axisY_sns, _axisY_duty, _axisY_rpm);
    fanForm->seriesFan()->addSeries(_chartFan);

    fanForm->seriesSensor()->setXAxis(_axisXFan);
    fanForm->seriesSensor()->setYAxis(_axisY_sns);
    fanForm->seriesSensor()->addSeries(_chartFan);

    fanForm->seriesFan()->setVisible(SeriesFan::SeriesType::setpoint, false);

    fanForm->seriesFan()->setName(SeriesFan::SeriesType::rpm, QString("%1 - RPM").arg(fanForm->dataFan()->channel() + 1));
    fanForm->seriesFan()->setName(SeriesFan::SeriesType::dutycycle, QString("%1 - Duty").arg(fanForm->dataFan()->channel() + 1));
    fanForm->seriesSensor()->setName(QString("%1 - Sensor").arg(fanForm->dataFan()->channel() + 1));

    on_showGraphUpdated(fanForm);

    /*
    _axisY->setRange(0.0, 75.0);
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
    */
}

void WidgetFanContainerForm::on_sensorValueUpdated(QWidget *fanWidget)
{
    Q_UNUSED(fanWidget);
}

void WidgetFanContainerForm::on_showGraphUpdated(QWidget *fanWidget)
{
    WidgetFanShowForm *fanShowForm = static_cast<WidgetFanShowForm *>(fanWidget);

    qDebug() << "WidgetFanContainerForm::on_showGraphUpdated " << fanShowForm->dataFan()->fullName();
    qDebug() << "WidgetFanContainerForm::on_showGraphUpdated " << fanShowForm->showGraphSensor();

    if (fanShowForm->showGraphSensor())
    {
        fanShowForm->on_SensorIndexChanged();
        fanShowForm->seriesSensor()->setXAxis(_axisXFan);
        fanShowForm->seriesSensor()->setYAxis(_axisY_sns);
        fanShowForm->seriesSensor()->addSeries(_chartFan);
    }

    fanShowForm->seriesFan()->setVisible(SeriesFan::SeriesType::rpm, fanShowForm->showGraphRpm());
    fanShowForm->seriesFan()->setVisible(SeriesFan::SeriesType::dutycycle, fanShowForm->showGraphDuty());
}

void WidgetFanContainerForm::on_colorGraphUpdated(QWidget *fanWidget)
{
    WidgetFanShowForm *fanShowForm = static_cast<WidgetFanShowForm *>(fanWidget);
    qDebug() << "on_colorGraphUpdated " << fanShowForm->dataFan()->fullName();

    int channel = fanShowForm->dataFan()->channel();
    qDebug() << "on_colorGraphUpdated " << channel;

    fanShowForm->seriesFan()->setColor(SeriesFan::SeriesType::rpm, fanShowForm->colorRpm());
    fanShowForm->seriesFan()->setColor(SeriesFan::SeriesType::dutycycle, fanShowForm->colorDuty());

    fanShowForm->seriesSensor()->setColor(fanShowForm->colorSensor());
}

void WidgetFanContainerForm::createFanChart()
{
    _axisY_sns = new QValueAxis();
    _axisY_sns->setLabelFormat("%d");
    _axisY_sns->setTitleText("Sensor Value");
    _axisY_sns->setTickCount(6);

    _axisY_duty = new QValueAxis();
    _axisY_duty->setLabelFormat("%d");
    _axisY_duty->setTitleText("Duty Cycle [%]");
    _axisY_duty->setTickCount(6);

    _axisY_rpm = new QValueAxis();
    _axisY_rpm->setLabelFormat("%d");
    _axisY_rpm->setTitleText("RPM [1/s]");
    _axisY_rpm->setTickCount(6);

    _axisXFan = new QDateTimeAxis;
    _axisXFan->setFormat("h:mm");

    QDateTime now = QDateTime::currentDateTime();
    _axisXFan->setRange(now.addSecs(-60*60), now);

    _chartFan = new QChart();
    _chartFan->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chartFan->addAxis(_axisY_sns, Qt::AlignRight);
    _chartFan->addAxis(_axisY_duty, Qt::AlignLeft);
    _chartFan->addAxis(_axisY_rpm, Qt::AlignLeft);

    _chartFan->setAxisX(_axisXFan);

    QChartView *chartView = new QChartView(_chartFan);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RubberBand::RectangleRubberBand);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _axisY_sns->setRange(0.0, 75.0);
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
}
