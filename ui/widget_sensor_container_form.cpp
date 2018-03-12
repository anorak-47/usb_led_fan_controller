#include "widget_sensor_container_form.h"
#include "ui_widget_sensor_container_form.h"
#include "widget_sensor_form.h"
#include "data_sensor.h"
#include "series_sensor.h"
#include "chart_settings_form.h"
#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>

QT_CHARTS_USE_NAMESPACE

WidgetSensorContainerForm::WidgetSensorContainerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSensorContainerForm)
{
    ui->setupUi(this);

    QWidget *w = new QWidget();

    _sensorLayout = new QVBoxLayout();
    w->setLayout(_sensorLayout);

    ui->sensorScrollArea->setWidget(w);

    _signalMapper = new QSignalMapper(this);
    connect(_signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(on_sensorValueUpdated(QWidget*)));

    _signalMapperShowGraph = new QSignalMapper(this);
    connect(_signalMapperShowGraph, SIGNAL(mapped(QWidget*)), this, SLOT(on_showGraphUpdated(QWidget*)));

    _signalMapperNameGraph = new QSignalMapper(this);
    connect(_signalMapperNameGraph, SIGNAL(mapped(QWidget*)), this, SLOT(on_nameGraphUpdated(QWidget*)));

    _signalMapperColorGraph = new QSignalMapper(this);
    connect(_signalMapperColorGraph, SIGNAL(mapped(QWidget*)), this, SLOT(on_colorGraphUpdated(QWidget*)));

    createSensorChart();
    //readSettings();
}

WidgetSensorContainerForm::~WidgetSensorContainerForm()
{
    saveSettings();
    delete ui;
}

void WidgetSensorContainerForm::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__;
    QSettings settings("Anorak", "ULFControl");
    settings.setValue("SensorContainer/splitter", ui->splitter->saveGeometry());

    int c = 0;
    for(int size : ui->splitter->sizes())
    {
        settings.setValue(QString("SensorContainer/splitter-size-%1").arg(c++), size);
    }
}

void WidgetSensorContainerForm::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    restoreSplitter();
}

void WidgetSensorContainerForm::restoreSplitter()
{
    QSettings settings("Anorak", "ULFControl");

    int c = 0;
    QList <int> sizes;
    for(int i = 0; i < ui->splitter->sizes().size(); i++)
    {
        int restoredSize = settings.value(QString("SensorContainer/splitter-size-%1").arg(c++), 350).toInt();
        sizes.append(restoredSize);
    }

    ui->splitter->setSizes(sizes);
}

void WidgetSensorContainerForm::createSensorChart()
{
    _axisY = new QValueAxis();
    _axisY->setLabelFormat("%d");
    _axisY->setTitleText("Sensor Value");

    _axisX = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisX->setFormat("h:mm");
    //axisX->setTitleText("Time");

    QDateTime now = QDateTime::currentDateTime();
    _axisX->setRange(now.addSecs(-_time_range_secs), now);
    _axisY->setRange(0.0, 75.0);

    _chart = new QChart();
    //_chart->legend()->hide();
    //_chart->setTitle("Sensors");
    _chart->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chart->setAxisY(_axisY);
    _chart->setAxisX(_axisX);

    QChartView *chartView = new QChartView(_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RubberBand::RectangleRubberBand);

    QVBoxLayout *cLayout = new QVBoxLayout;
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    //chartView->layout()->setContentsMargins(0, 0, 0, 0);

    cLayout->addWidget(chartView);

    ChartSettingsForm *csf = new ChartSettingsForm();
    cLayout->addWidget(csf);

    connect(csf, SIGNAL(signalPause(bool)), this, SLOT(on_pause(bool)));
    connect(csf, SIGNAL(signalTimeRangeChanged(QTime)), this, SLOT(on_timeRangeChanged(QTime)));
}

void WidgetSensorContainerForm::on_currentTabChanged(int index)
{
    Q_UNUSED(index);

    qDebug() << "WidgetSensorContainerForm: visible: " << isVisible();

}

void WidgetSensorContainerForm::addWidgetSensor(WidgetSensorForm *sensor, bool hasSuccessor)
{
    _widgetSensorForms.push_back(sensor);

    _sensorLayout->addWidget(sensor);

    if (hasSuccessor)
    {
        QFrame *line = new QFrame(this);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _sensorLayout->addWidget(line);
    }

    connect(sensor, SIGNAL(signalValueUpdated()), _signalMapper, SLOT(map()));
    _signalMapper->setMapping(sensor, sensor);

    connect(sensor, SIGNAL(signalShowGraphChanged()), _signalMapperShowGraph, SLOT(map()));
    _signalMapperShowGraph->setMapping(sensor, sensor);

    connect(sensor, SIGNAL(signalGraphNameChanged()), _signalMapperNameGraph, SLOT(map()));
    _signalMapperNameGraph->setMapping(sensor, sensor);

    connect(sensor, SIGNAL(signalGraphColorChanged()), _signalMapperColorGraph, SLOT(map()));
    _signalMapperColorGraph->setMapping(sensor, sensor);

    addSensorSeriesToGraph(sensor);
}

void WidgetSensorContainerForm::on_sensorValueUpdated(QWidget *sensorWidget)
{	
    Q_UNUSED(sensorWidget);
    //WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    //qDebug() << "on_sensorValueUpdated " << sensorFromWidget->dataSensor()->fullName();

    /*
    QLineSeries *lineSeries = _lineSeries[sensorWidget];
    QContiguousCache<TimeSeriesData> series = sensorFromWidget->dataSensor()->series();

    lineSeries->append(series.last().dt.toMSecsSinceEpoch(), series.last().value);
    */
}

QString WidgetSensorContainerForm::getNameForSeries(WidgetSensorForm *sensorFormWidget)
{
    //return QString("%1 - %2").arg(sensorFromWidget->dataSensor()->channel() + 1).arg(sensorFromWidget->dataSensor()->description());
    return QString("%1 - %2").arg(sensorFormWidget->dataSensor()->channel() + 1).arg(sensorFormWidget->getDescription());
}

void WidgetSensorContainerForm::addSensorSeriesToGraph(WidgetSensorForm *sensorFormWidget)
{
    std::shared_ptr<SeriesSensor> series = sensorFormWidget->seriesSensor();

    series->setXAxis(_axisX);
    series->setYAxis(_axisY);
    series->addSeries(_chart);

	/*
    QDateTime xmin = _axisX->min();
    QLineSeries *newLineSeries = new QLineSeries();
    newLineSeries->setName(getNameForSeries(sensorFromWidget));
    newLineSeries->setColor(sensorFromWidget->getGraphColor());

    _lineSeries.insert(std::pair<QWidget*, QT_CHARTS_NAMESPACE::QLineSeries *>(sensorFromWidget, newLineSeries));

    QContiguousCache<TimeSeriesData> series = sensorFromWidget->dataSensor()->series();

    for (int i = 0; i < series.count(); i++)
        if (series[i].dt > xmin)
            newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);

    _chart->addSeries(newLineSeries);

    newLineSeries->attachAxis(_axisX);
    newLineSeries->attachAxis(_axisY);
    */
}

void WidgetSensorContainerForm::on_showGraphUpdated(QWidget *sensorWidget)
{
    if (_chart_paused)
        return;

    WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    qDebug() << "on_showGraphUpdated " << sensorFromWidget->dataSensor()->fullName();

    /*
    auto it = _lineSeries.find(sensorWidget);
    if (it != _lineSeries.end())
    {
        QLineSeries *lineSeries = it->second;

        _chart->removeSeries(lineSeries);
        _lineSeries.erase(it);
    }

    if (sensorFromWidget->showInGraph())
    {
        addSensorSeriesToGraph(sensorFromWidget);
    }
    */
}

void WidgetSensorContainerForm::on_nameGraphUpdated(QWidget *sensorWidget)
{
    Q_UNUSED(sensorWidget);
    //WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    /*
    auto it = _lineSeries.find(sensorWidget);
    if (it != _lineSeries.end())
    {
           QLineSeries *lineSeries = it->second;
           lineSeries->setName(getNameForSeries(sensorFromWidget));
    }
    */
}

void WidgetSensorContainerForm::on_colorGraphUpdated(QWidget *sensorWidget)
{
    Q_UNUSED(sensorWidget);
    //WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    /*
    auto it = _lineSeries.find(sensorWidget);
    if (it != _lineSeries.end())
    {
           QLineSeries *lineSeries = it->second;
           lineSeries->setColor(sensorFromWidget->getGraphColor());
    }
    */
}

void WidgetSensorContainerForm::on_timeRangeChanged(QTime timerange)
{
    _time_range_secs = QTime(0, 0).secsTo(timerange);

    QDateTime now = QDateTime::currentDateTime();
    _axisX->setRange(now.addSecs(-_time_range_secs), now);
}

void WidgetSensorContainerForm::on_pause(bool paused)
{
    _chart_paused = paused;

    if (!paused)
    {
        _chart->setTitle(tr("Sensors"));
        for (auto &sensor : _widgetSensorForms)
        {
            if (sensor->showInGraph())
            {
                on_showGraphUpdated(sensor);
            }
        }
    }
    else
    {
        _chart->setTitle(tr("Sensors - Update Paused"));
    }
}
