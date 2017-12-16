#include "widget_fan_container_form.h"
#include "ui_widget_fan_container_form.h"
#include "widget_fan_show_form.h"
#include "config.h"
#include "data_fan.h"
#include "data_sensor.h"
#include <QtGui/QLayout>
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

    _seriesFanRpm.resize(MAX_FANS, 0);
    _seriesFanDuty.resize(MAX_FANS, 0);
    _seriesFanSensor.resize(MAX_FANS, 0);

    createFanChart();
    readSettings();
}

WidgetFanContainerForm::~WidgetFanContainerForm()
{
    saveSettings();
    delete ui;
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
    //ui->splitter->restoreGeometry(settings.value("FanContainer/splitter").toByteArray());
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

    on_showGraphUpdated(fanForm);
}

void WidgetFanContainerForm::on_sensorValueUpdated(QWidget *fanWidget)
{
    WidgetFanShowForm *fanShowForm = static_cast<WidgetFanShowForm *>(fanWidget);
    qDebug() << "on_sensorValueUpdated " << fanShowForm->dataFan()->fullName();

    int channel = fanShowForm->dataFan()->channel();
    qDebug() << "on_sensorValueUpdated " << channel;


    if (fanShowForm->showGraphRpm())
    {
        QLineSeries *lineSeries = _seriesFanRpm[channel];
        if (lineSeries)
        {
            QContiguousCache<TimeSeriesData> series = fanShowForm->dataFan()->seriesRpm();
            lineSeries->append(series.last().dt.toMSecsSinceEpoch(), series.last().value);
        }
    }

    if (fanShowForm->showGraphDuty())
    {
        QLineSeries *lineSeries = _seriesFanDuty[channel];
        if (lineSeries)
        {
            QContiguousCache<TimeSeriesData> series = fanShowForm->dataFan()->seriesDuty();
            lineSeries->append(series.last().dt.toMSecsSinceEpoch(), series.last().value);
        }
    }

    if (fanShowForm->showGraphSensor())
    {
        QLineSeries *lineSeries = _seriesFanSensor[channel];
        if (lineSeries)
        {
            QContiguousCache<TimeSeriesData> series = fanShowForm->dataSensor()->series();
            lineSeries->append(series.last().dt.toMSecsSinceEpoch(), series.last().value);
        }
    }
}

void WidgetFanContainerForm::on_showGraphUpdated(QWidget *fanWidget)
{
    WidgetFanShowForm *fanShowForm = static_cast<WidgetFanShowForm *>(fanWidget);
    qDebug() << "on_showGraphUpdated " << fanShowForm->dataFan()->fullName();

    int channel = fanShowForm->dataFan()->channel();
    qDebug() << "on_showGraphUpdated " << channel;

    if (fanShowForm->showGraphRpm())
    {
        QLineSeries *newLineSeries;

        if (!_seriesFanRpm[channel])
        {
            newLineSeries = new QLineSeries();
            newLineSeries->setName(QString("%1 - RPM").arg(channel+1));
            newLineSeries->setColor(fanShowForm->colorRpm());
            _seriesFanRpm[channel] = newLineSeries;

            _chartFan->addSeries(newLineSeries);
            newLineSeries->attachAxis(_axisXFan);
            newLineSeries->attachAxis(_axisY_rpm);
        }
        else
        {
            newLineSeries = _seriesFanRpm[channel];
        }

        newLineSeries->clear();

        QDateTime xmin = _axisXFan->min();
        QContiguousCache<TimeSeriesData> series = fanShowForm->dataFan()->seriesRpm();

        for (int i = 0; i < series.count(); i++)
            if (series[i].dt > xmin)
                newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);

        newLineSeries->show();
    }
    else
    {
        if (_seriesFanRpm[channel])
            _seriesFanRpm[channel]->hide();
    }

    if (fanShowForm->showGraphDuty())
    {
        QLineSeries *newLineSeries;

        if (!_seriesFanDuty[channel])
        {
            newLineSeries = new QLineSeries();
            newLineSeries->setName(QString("%1 - Duty").arg(channel+1));
            newLineSeries->setColor(fanShowForm->colorDuty());
            _seriesFanDuty[channel] = newLineSeries;

            _chartFan->addSeries(newLineSeries);
            newLineSeries->attachAxis(_axisXFan);
            newLineSeries->attachAxis(_axisY_duty);
        }
        else
        {
            newLineSeries = _seriesFanDuty[channel];
        }

        newLineSeries->clear();

        QDateTime xmin = _axisXFan->min();
        QContiguousCache<TimeSeriesData> series = fanShowForm->dataFan()->seriesDuty();

        for (int i = 0; i < series.count(); i++)
            if (series[i].dt > xmin)
                newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);

        newLineSeries->show();
    }
    else
    {
        if (_seriesFanDuty[channel])
            _seriesFanDuty[channel]->hide();
    }

    if (fanShowForm->showGraphSensor())
    {
        QLineSeries *newLineSeries;

        if (!_seriesFanSensor[channel])
        {
            newLineSeries = new QLineSeries();
            newLineSeries->setName(QString("%1 - Sensor").arg(channel+1));
            newLineSeries->setColor(fanShowForm->colorSensor());
            _seriesFanSensor[channel] = newLineSeries;

            _chartFan->addSeries(newLineSeries);
            newLineSeries->attachAxis(_axisXFan);
            newLineSeries->attachAxis(_axisY);
        }
        else
        {
            newLineSeries = _seriesFanSensor[channel];
        }

        newLineSeries->clear();

        QDateTime xmin = _axisXFan->min();
        QContiguousCache<TimeSeriesData> series = fanShowForm->dataSensor()->series();

        for (int i = 0; i < series.count(); i++)
            if (series[i].dt > xmin)
                newLineSeries->append(series[i].dt.toMSecsSinceEpoch(), series[i].value);

        newLineSeries->show();
    }
    else
    {
        if (_seriesFanSensor[channel])
            _seriesFanSensor[channel]->hide();
    }

    _axisY->setRange(0.0, 50.0);
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
}

void WidgetFanContainerForm::on_colorGraphUpdated(QWidget *fanWidget)
{
    qDebug() << __PRETTY_FUNCTION__;

    WidgetFanShowForm *fanShowForm = static_cast<WidgetFanShowForm *>(fanWidget);
    qDebug() << "on_colorGraphUpdated " << fanShowForm->dataFan()->fullName();

    int channel = fanShowForm->dataFan()->channel();
    qDebug() << "on_colorGraphUpdated " << channel;

    if (_seriesFanRpm[channel])
        if (_seriesFanRpm[channel]->color() != fanShowForm->colorRpm())
            _seriesFanRpm[channel]->setColor(fanShowForm->colorRpm());

    if (_seriesFanDuty[channel])
        if (_seriesFanDuty[channel]->color() != fanShowForm->colorDuty())
            _seriesFanDuty[channel]->setColor(fanShowForm->colorDuty());

    if (_seriesFanSensor[channel])
        if (_seriesFanSensor[channel]->color() != fanShowForm->colorSensor())
            _seriesFanSensor[channel]->setColor(fanShowForm->colorSensor());
}

void WidgetFanContainerForm::createFanChart()
{
    _axisY = new QValueAxis();
    _axisY->setLabelFormat("%d");
    _axisY->setTitleText("Value");
    _axisY->setTickCount(6);

    _axisY_duty = new QValueAxis();
    _axisY_duty->setLabelFormat("%d");
    _axisY_duty->setTitleText("Duty Cycle [%]");
    _axisY_duty->setTickCount(6);

    _axisY_rpm = new QValueAxis();
    _axisY_rpm->setLabelFormat("%d");
    _axisY_rpm->setTitleText("RPM [1/s]");
    _axisY_rpm->setTickCount(6);

    _axisXFan = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisXFan->setFormat("h:mm");
    //axisX_fan->setTitleText("Time");

    QDateTime now = QDateTime::currentDateTime();
    _axisXFan->setRange(now.addSecs(-60*60), now);

    _chartFan = new QChart();
    //fanChart->legend()->hide();
    //_chartFan->setTitle(tr("Fans"));
    _chartFan->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chartFan->addAxis(_axisY, Qt::AlignLeft);
    _chartFan->addAxis(_axisY_duty, Qt::AlignLeft);
    _chartFan->addAxis(_axisY_rpm, Qt::AlignRight);

    _chartFan->setAxisX(_axisXFan);

    QChartView *chartView = new QChartView(_chartFan);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RubberBand::RectangleRubberBand);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _axisY->setRange(0.0, 50.0);
    _axisY_duty->setRange(0.0, 100.0);
    _axisY_rpm->setRange(0.0, 5000.0);
}
