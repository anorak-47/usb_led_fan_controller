	#include "widget_fanout_form.h"
#include "ui_widget_fanout_form.h"
#include "data_fan_out.h"
#include "data_fan.h"
#include "series_fanout.h"
#include "usbface.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <math.h>

QT_CHARTS_USE_NAMESPACE

WidgetFanOutForm::WidgetFanOutForm(std::shared_ptr<DataFanOut> dataFanOut, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFanOutForm),
    _dataFanOut(dataFanOut)
{
    ui->setupUi(this);
    _seriesFanOut = std::shared_ptr<SeriesFanOut>(new SeriesFanOut(_dataFanOut));

    _stallDetectionFans.resize(MAX_FANS);
    _stallDetectionFans[0] = ui->cbDetectFan0;
    _stallDetectionFans[1] = ui->cbDetectFan1;
    _stallDetectionFans[2] = ui->cbDetectFan2;
    _stallDetectionFans[3] = ui->cbDetectFan3;
    _stallDetectionFans[4] = ui->cbDetectFan4;
    _stallDetectionFans[5] = ui->cbDetectFan5;

    _stalledFans.resize(MAX_FANS);
    _stalledFans[0] = ui->fsFan0;
    _stalledFans[1] = ui->fsFan1;
    _stalledFans[2] = ui->fsFan2;
    _stalledFans[3] = ui->fsFan3;
    _stalledFans[4] = ui->fsFan4;
    _stalledFans[5] = ui->fsFan5;

    ui->lTitle->setText(_dataFanOut->fullName());

    QSignalBlocker sp(ui->spRpmOut);
    ui->spRpmOut->setValue(dataFanOut->data().rpm);

    showFanOutModes();

    on_supportedFunctionsUpdated(SUPPORTED_NONE);

    connect(_dataFanOut.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataFanOut.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));

    createChart();
}

WidgetFanOutForm::~WidgetFanOutForm()
{
    delete ui;
}

void WidgetFanOutForm::showFanOutModes()
{
    int index = 0;
    QSignalBlocker cb(ui->comboBox);
    for (int i = FANOUTMODE_RPS_MINFAN; i < FANOUTMODE_MAX; i++)
    {
        QString typeString = usbfaceFanOutModeToString((FANOUTMODE)i);
        if (!typeString.isEmpty())
        {
            ui->comboBox->addItem(typeString, i);
            _modeToIndex[(FANOUTMODE)i] = index;
            index++;
        }
    }
}

void WidgetFanOutForm::setDataFans(std::vector<std::shared_ptr<DataFan> > dataFans)
{
    _dataFans = dataFans;
}

void WidgetFanOutForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
}

void WidgetFanOutForm::on_supportedFunctionsUpdated(int supportedFunctions)
{
    this->setEnabled((supportedFunctions & SUPPORTED_FAN_OUT));
}

void WidgetFanOutForm::on_comboBox_currentIndexChanged(int index)
{
    _dataFanOut->updateMode((FANOUTMODE)ui->comboBox->itemData(index).toInt());
    ui->lTitle->setText(_dataFanOut->fullName());
}

void WidgetFanOutForm::showStalledFans()
{
    for (auto &fan : _dataFans)
    {
        if (fan->channel() < _stalledFans.size())
        {
            _stalledFans[fan->channel()]->setFanIsStalled(fan->data().status.stalled);
        }
    }
}

void WidgetFanOutForm::on_dataUpdated()
{
    QSignalBlocker cb(ui->comboBox);
    ui->comboBox->setCurrentIndex(_dataFanOut->data().mode);
    QSignalBlocker sp(ui->spRpmOut);
    ui->spRpmOut->setValue(_dataFanOut->data().rpm);

    for (int i = 0; i < _stallDetectionFans.size(); i++)
    {
        QSignalBlocker b(_stallDetectionFans[i]);
        _stallDetectionFans[i]->setChecked(_dataFanOut->data().fanStallDetect & (1 << i));
    }

    showStalledFans();
}

void WidgetFanOutForm::showDutyCycle()
{
    int fanid = _dataFanOut->data().mode;
    if (fanid >= FANOUTMODE_RPS_CLONEFAN0 && fanid <= FANOUTMODE_RPS_CLONEFAN5)
    {
        fanid -= FANOUTMODE_RPS_CLONEFAN0;
        for (auto &fan : _dataFans)
        {
            if (fan->channel() == fanid)
            {
                ui->spDutyCycleIn->setValue(round(fan->getScaledDuty()*100.0));
                break;
            }
        }
    }
    else if (fanid == FANOUTMODE_RPS_FIXED_50)
    {
        ui->spDutyCycleIn->setValue(50);
    }
    else if (fanid == FANOUTMODE_RPS_MINFAN)
    {
        int min = 100000;
        double duty = 0.0;
        for (auto &fan : _dataFans)
        {
            if (min > fan->getRPM())
            {
                min = fan->getRPM();
                duty = fan->getScaledDuty()*100.0;
            }
        }

        ui->spDutyCycleIn->setValue(round(duty));
    }
    else if (fanid == FANOUTMODE_RPS_MAXFAN)
    {
        int max = 0;
        double duty = 0.0;
        for (auto &fan : _dataFans)
        {
            if (max < fan->getRPM())
            {
                max = fan->getRPM();
                duty = fan->getScaledDuty()*100.0;
            }
        }

        ui->spDutyCycleIn->setValue(round(duty));
    }
    else
    {
        ui->spDutyCycleIn->setValue(0);
    }
}

void WidgetFanOutForm::on_valueUpdated()
{
    //qDebug() << "on_dataUpdated " << _dataFanOut->name();
    QSignalBlocker sp(ui->spRpmOut);
    ui->spRpmOut->setValue(_dataFanOut->data().rpm);

    showDutyCycle();
    showStalledFans();
}

void WidgetFanOutForm::createChart()
{
    QChartView *chartView = _seriesFanOut->createChartView();

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    /*
    QValueAxis *axisY_duty = new QValueAxis();
    axisY_duty->setLabelFormat("%d");
    axisY_duty->setTitleText("Duty Cycle [%]");

    QValueAxis *axisY_rpm = new QValueAxis();
    axisY_rpm->setLabelFormat("%d");
    axisY_rpm->setTitleText("RPM [1/s]");

    _axisXFan = new QDateTimeAxis;
    //axisX->setFormat("dd-MM-yyyy h:mm");
    _axisXFan->setFormat("h:mm");
    //axisX_fan->setTitleText("Time");

    QDateTime now = QDateTime::currentDateTime();
    _axisXFan->setRange(now.addSecs(-60*60), now);

    _chartFan = new QChart();
    //fanChart->legend()->hide();
    //_chartFan->setTitle(tr("Fan Output"));
    _chartFan->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chartFan->addAxis(axisY_duty, Qt::AlignLeft);
    _chartFan->addAxis(axisY_rpm, Qt::AlignRight);

    _chartFan->setAxisX(_axisXFan);

    QChartView *chartView = new QChartView(_chartFan);
    chartView->setRenderHint(QPainter::Antialiasing);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _seriesFanRpm = new QLineSeries();
    _seriesFanRpm->setName("Fan RPM");
    _chartFan->addSeries(_seriesFanRpm);
    _seriesFanRpm->attachAxis(_axisXFan);
    _seriesFanRpm->attachAxis(axisY_rpm);

    _seriesFanDuty = new QLineSeries();
    _seriesFanDuty->setName("Fan Duty Cycle");
    _chartFan->addSeries(_seriesFanDuty);
    _seriesFanDuty->attachAxis(_axisXFan);
    _seriesFanDuty->attachAxis(axisY_duty);

    axisY_duty->setRange(0.0, 100.0);
    axisY_rpm->setRange(0.0, 5000.0);
    */
}
