#include "widget_sensor_container_form.h"
#include "ui_widget_sensor_container_form.h"
#include "widget_sensor_form.h"
#include "data_sensor.h"
#include "chart_settings_form.h"
#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QtGui/QLayout>
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





    //QTimer *timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    //timer->start(1000);






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
    //ui->splitter->restoreGeometry(settings.value("SensorContainer/splitter").toByteArray());
    restoreSplitter();
}

void WidgetSensorContainerForm::restoreSplitter()
{
    QSettings settings("Anorak", "ULFControl");

    int c = 0;
    QList <int> sizes;
    for(int size : ui->splitter->sizes())
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
    _axisY->setRange(0.0, 100.0);

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

    //if (sensor->dataSensor()->channel() == 0 || sensor->dataSensor()->channel() == 1)
    {

        /*
        QLineSeries *newLineSeries = new QLineSeries();
        newLineSeries->setName(sensor->dataSensor()->name());
        _lineSeries.insert(std::pair<QWidget*, QT_CHARTS_NAMESPACE::QLineSeries *>(sensor, newLineSeries));



        QContiguousCache<TimeSeriesData> series = sensor->dataSensor()->series();
        qDebug() << "add series data " << series.count();


        for (int i = 0; i < series.count(); i++)
        {
            qDebug() << "  " << series[i].dt.toMSecsSinceEpoch() << " x " << series[i].value;
            newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);
        }

        _chart->addSeries(newLineSeries);

        newLineSeries->attachAxis(axisX);
        newLineSeries->hide();
        */

        if (sensor->showInGraph())
        {
            addSensorSeriesToGraph(sensor);
            /*
            QLineSeries *newLineSeries = new QLineSeries();
            newLineSeries->setName(QString("%1 - %2").arg(sensor->dataSensor()->channel()).arg(sensor->dataSensor()->description()));
            _lineSeries.insert(std::pair<QWidget*, QT_CHARTS_NAMESPACE::QLineSeries *>(sensor, newLineSeries));

            _firstLineSeries = newLineSeries;

            QContiguousCache<TimeSeriesData> series = sensor->dataSensor()->series();
            qDebug() << "add series data " << series.count();

            for (int i = 0; i < series.count(); i++)
            {
                newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);
            }

            _chart->addSeries(newLineSeries);

            newLineSeries->attachAxis(axisX);
            */
        }
    }
}

void WidgetSensorContainerForm::on_sensorValueUpdated(QWidget *sensorWidget)
{
    WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    qDebug() << "on_sensorValueUpdated " << sensorFromWidget->dataSensor()->fullName();

    QLineSeries *lineSeries = _lineSeries[sensorWidget];
    QContiguousCache<TimeSeriesData> series = sensorFromWidget->dataSensor()->series();

    lineSeries->append(series.last().dt.toMSecsSinceEpoch(), series.last().value);
}

QString WidgetSensorContainerForm::getNameForSeries(WidgetSensorForm *sensorFromWidget)
{
    //return QString("%1 - %2").arg(sensorFromWidget->dataSensor()->channel() + 1).arg(sensorFromWidget->dataSensor()->description());
    return QString("%1 - %2").arg(sensorFromWidget->dataSensor()->channel() + 1).arg(sensorFromWidget->getDescription());
}

void WidgetSensorContainerForm::addSensorSeriesToGraph(WidgetSensorForm *sensorFromWidget)
{
    QDateTime xmin = _axisX->min();
    QLineSeries *newLineSeries = new QLineSeries();
    newLineSeries->setName(getNameForSeries(sensorFromWidget));
    newLineSeries->setColor(sensorFromWidget->getGraphColor());
    _lineSeries.insert(std::pair<QWidget*, QT_CHARTS_NAMESPACE::QLineSeries *>(sensorFromWidget, newLineSeries));



    _firstLineSeries = newLineSeries;



    QContiguousCache<TimeSeriesData> series = sensorFromWidget->dataSensor()->series();

    for (int i = 0; i < series.count(); i++)
        if (series[i].dt > xmin)
            newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);

    _chart->addSeries(newLineSeries);

    newLineSeries->attachAxis(_axisX);
    newLineSeries->attachAxis(_axisY);
}

void WidgetSensorContainerForm::on_showGraphUpdated(QWidget *sensorWidget)
{
    if (_chart_paused)
        return;

    WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    qDebug() << "on_showGraphUpdated " << sensorFromWidget->dataSensor()->fullName();

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
}

void WidgetSensorContainerForm::on_nameGraphUpdated(QWidget *sensorWidget)
{
    WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    auto it = _lineSeries.find(sensorWidget);
    if (it != _lineSeries.end())
    {
           QLineSeries *lineSeries = it->second;
           lineSeries->setName(getNameForSeries(sensorFromWidget));
    }
}

void WidgetSensorContainerForm::on_colorGraphUpdated(QWidget *sensorWidget)
{
    WidgetSensorForm *sensorFromWidget = static_cast<WidgetSensorForm *>(sensorWidget);

    auto it = _lineSeries.find(sensorWidget);
    if (it != _lineSeries.end())
    {
           QLineSeries *lineSeries = it->second;
           lineSeries->setColor(sensorFromWidget->getGraphColor());
    }
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

void WidgetSensorContainerForm::on_timeout()
{
    if (_chart_paused)
        return;

    qDebug() << "on_timeout";
    //qDebug() << "w " << _chart->plotArea().width();
    //qDebug() << "t " << axisX->tickCount();

    if (!_firstLineSeries || _firstLineSeries->count() < 2)
        return;

    QDateTime now = QDateTime::currentDateTime();

    QDateTime xmin = _axisX->min();
    QDateTime xmax = _axisX->max();


    /*
    if (xmin.secsTo(now) < 60*60)
    {
        xmax = now;
    }
    else
    {
        xmax = now;
        xmin = now.addSecs(-60*60);
    }
    */

    xmax = now;

    //qreal x = _chart->plotArea().width() / axisX->tickCount();
    //qDebug() << "x " << x;

    double v = _firstLineSeries->at(_firstLineSeries->count()-1).y();
    v += random() % 2 ? +random() % 2 : -random() % 2;

    _firstLineSeries->append(now.toMSecsSinceEpoch(), v);
    //_chart->scroll(x, 0);

    _axisX->setRange(now.addSecs(-_time_range_secs), now);
    _axisY->setRange(-20, 20);
}
