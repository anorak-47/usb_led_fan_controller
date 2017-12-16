#include "widget_fan_form.h"
#include "ui_widget_fan_form.h"
#include "data_fan.h"
#include "data_sensor.h"
#include "usbface.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QSettings>
#include <QtCore/QDebug>

QT_CHARTS_USE_NAMESPACE

WidgetFanForm::WidgetFanForm(std::shared_ptr<DataFan> dataFan, QWidget *parent) :
    QWidget(parent), ui(new Ui::WidgetFanForm), _dataFan(dataFan)
{
    ui->setupUi(this);

    showFanTypes();
    on_supportedFunctionsUpdated(SUPPORTED_NONE | SUPPORTED_FANMODE_LINEAR | SUPPORTED_FANMODE_TP| SUPPORTED_FANMODE_LIN_TP);

    indexSensorLinear = dataFan->data().config.snsIdx;
    indexSensorPiController = dataFan->data().config.snsIdx;
    indexSensorTripPoints = dataFan->data().config.snsIdx;

    connect(_dataFan.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataFan.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));




    _dataFan->data().trip_point[0].value = 20;
    _dataFan->data().trip_point[1].value = 35;
    _dataFan->data().trip_point[2].value = 50;
    _dataFan->data().trip_point[3].value = 60;
    _dataFan->data().trip_point[4].value = 100;

    _dataFan->data().trip_point[0].duty = 0;
    _dataFan->data().trip_point[1].duty = 25;
    _dataFan->data().trip_point[2].duty = 40;
    _dataFan->data().trip_point[3].duty = 75;
    _dataFan->data().trip_point[4].duty = 90;




    createLinearControllerChart();
    createTripPointControllerChart();
    createFanChart();

    readSettings();
    setTitle(dataFan);
}

WidgetFanForm::~WidgetFanForm()
{
    saveSettings();
    delete ui;
}

void WidgetFanForm::setTitle(std::shared_ptr<DataFan> dataFan)
{
    QString titleText = dataFan->fullName();
    QString description = ui->leDescription->text();
    if (!description.isEmpty())
        titleText.prepend(description + " - ");
    ui->lTitle->setText(titleText);
}

void WidgetFanForm::saveSettings()
{
    QSettings settings("Anorak", "ULFControl");
    settings.setValue(QString("Fan%1/description").arg(_dataFan->channel()), ui->leDescription->text());
}

void WidgetFanForm::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    ui->leDescription->setText(settings.value(QString("Fan%1/description").arg(_dataFan->channel())).toString());
}

void WidgetFanForm::createFanChart()
{
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Value");

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
    //_chartFan->setTitle(_dataFan->fullName());
    _chartFan->setTheme(QChart::ChartTheme::ChartThemeBlueNcs);

    _chartFan->addAxis(axisY, Qt::AlignLeft);
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

    _seriesFanSetpoint = new QLineSeries();
    _seriesFanSetpoint->setName("PI Setpoint");
    _chartFan->addSeries(_seriesFanSetpoint);
    _seriesFanSetpoint->attachAxis(_axisXFan);
    _seriesFanSetpoint->attachAxis(axisY);

    _seriesFanSensor = new QLineSeries();
    _seriesFanSensor->setName("Sensor");
    _chartFan->addSeries(_seriesFanSensor);
    _seriesFanSensor->attachAxis(_axisXFan);
    _seriesFanSensor->attachAxis(axisY);

    axisY->setRange(0.0, 50.0);
    axisY_duty->setRange(0.0, 100.0);
    axisY_rpm->setRange(0.0, 5000.0);
}

void WidgetFanForm::updateTripPointControllerChart()
{
    if (_dataFan->data().config.fanMode == FANMODE_LINEAR_TRIP_POINTS)
    {
        for (int i = 0; i < MAX_TRIP_POINTS; i++)
        {
            qDebug() << "- i: " << i;
            _seriesTripPointController->replace(i, 0.0, 0.0);
        }

        for (int i = 0; i < MAX_TRIP_POINTS; i++)
        {
            qDebug() << "+ i: " << i+MAX_TRIP_POINTS;
            _seriesTripPointController->replace(i+MAX_TRIP_POINTS, _dataFan->data().trip_point[i].value, _dataFan->data().trip_point[i].duty);
        }
    }
    else
    {
        _seriesTripPointController->replace(0, 0.0, _dataFan->data().trip_point[0].duty);
        _seriesTripPointController->replace(1, _dataFan->data().trip_point[0].value, _dataFan->data().trip_point[0].duty);

        for (int i = 1; i < MAX_TRIP_POINTS-1; i++)
        {
            _seriesTripPointController->replace(i*2, _dataFan->data().trip_point[i-1].value, _dataFan->data().trip_point[i].duty);
            _seriesTripPointController->replace(i*2+1, _dataFan->data().trip_point[i].value, _dataFan->data().trip_point[i].duty);
        }

        _seriesTripPointController->replace((MAX_TRIP_POINTS-1)*2, _dataFan->data().trip_point[(MAX_TRIP_POINTS-1)-1].value, _dataFan->data().trip_point[MAX_TRIP_POINTS-1].duty);
        _seriesTripPointController->replace((MAX_TRIP_POINTS-1)*2+1, _dataFan->data().trip_point[(MAX_TRIP_POINTS-1)].value, _dataFan->data().trip_point[MAX_TRIP_POINTS-1].duty);
    }

    //axisX_tripPoint->setRange(0.0, 80.0);
}

void WidgetFanForm::createTripPointControllerChart()
{
    QChart *tripPointControllerChart = new QChart();
    tripPointControllerChart->legend()->hide();
    //tripPointControllerChart->setTitle("Trip Point Controller");
    tripPointControllerChart->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);

    QChartView *chartView = new QChartView(tripPointControllerChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wTripPointChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    _seriesTripPointController = new QLineSeries();
    _seriesTripPointController->setName("Trip Points");
    _seriesTripPointController->setPointsVisible(true);
    _seriesTripPointController->setPointLabelsVisible(true);
    _seriesTripPointController->setPointLabelsFormat("@xPoint/@yPoint");

    _seriesTripPointController->append(0.0, 0.0);

    for (int i = 1; i < MAX_TRIP_POINTS; i++)
    {
        _seriesTripPointController->append(_dataFan->data().trip_point[i].value, _dataFan->data().trip_point[i].duty);
        _seriesTripPointController->append(_dataFan->data().trip_point[i+1].value, _dataFan->data().trip_point[i+1].duty);
    }

    _seriesTripPointController->append(100.0, 100.0);

    tripPointControllerChart->addSeries(_seriesTripPointController);
    tripPointControllerChart->createDefaultAxes();

    Q_ASSERT(tripPointControllerChart->axisX()->type() == QAbstractAxis::AxisType::AxisTypeValue);
    Q_ASSERT(tripPointControllerChart->axisY()->type() == QAbstractAxis::AxisType::AxisTypeValue);

    static_cast<QValueAxis*>(tripPointControllerChart->axisX())->setLabelFormat("%d");
    static_cast<QValueAxis*>(tripPointControllerChart->axisX())->setTickCount(9);
    static_cast<QValueAxis*>(tripPointControllerChart->axisY())->setLabelFormat("%d");
    static_cast<QValueAxis*>(tripPointControllerChart->axisY())->setTickCount(6);
    tripPointControllerChart->axisX()->setRange(0, 80);
    tripPointControllerChart->axisY()->setRange(0, 100);
}

void WidgetFanForm::updateLinearControllerChart()
{
    _seriesLinearController->replace(0, 0.0, getLinearControllerPoint(0.0));
    _seriesLinearController->replace(1, 100.0, getLinearControllerPoint(100.0));
}

void WidgetFanForm::createLinearControllerChart()
{
    QChart *linearControllerChart = new QChart();
    linearControllerChart->legend()->hide();
    //linearControllerChart->setTitle("Linear Controller");
    linearControllerChart->setTheme(QChart::ChartTheme::ChartThemeBlueCerulean);

    QChartView *chartView = new QChartView(linearControllerChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QGridLayout *cLayout = new QGridLayout();
    cLayout->setContentsMargins(0, 0, 0, 0);
    ui->wLinearChart->setLayout(cLayout);
    cLayout->addWidget(chartView);

    QLineSeries *seriesBase = new QLineSeries();
    seriesBase->setName("Linear");
    seriesBase->append(0.0, 0.0);
    seriesBase->append(100.0, 100.0);

    _seriesLinearController = new QLineSeries();
    _seriesLinearController->setName("Settings");
    _seriesLinearController->append(0.0, getLinearControllerPoint(0.0));
    _seriesLinearController->append(100.0, getLinearControllerPoint(100.0));

    linearControllerChart->addSeries(seriesBase);
    linearControllerChart->addSeries(_seriesLinearController);
    linearControllerChart->createDefaultAxes();

    Q_ASSERT(linearControllerChart->axisX()->type() == QAbstractAxis::AxisType::AxisTypeValue);
    Q_ASSERT(linearControllerChart->axisY()->type() == QAbstractAxis::AxisType::AxisTypeValue);

    static_cast<QValueAxis*>(linearControllerChart->axisX())->setLabelFormat("%d");
    static_cast<QValueAxis*>(linearControllerChart->axisX())->setTickCount(9);
    static_cast<QValueAxis*>(linearControllerChart->axisY())->setLabelFormat("%d");
    static_cast<QValueAxis*>(linearControllerChart->axisY())->setTickCount(6);
    linearControllerChart->axisX()->setRange(0, 80);
    linearControllerChart->axisY()->setRange(0, 100);
}
void WidgetFanForm::updateSpinBox(QSpinBox *box, int value)
{
    QSignalBlocker sb(box);
    box->setValue(value);
}

void WidgetFanForm::updateDoubleSpinBox(QDoubleSpinBox *box, double value)
{
    QSignalBlocker sb(box);
    box->setValue(value);
}

void WidgetFanForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
    updateSensorSelectors();
    on_dataUpdated();
}

void WidgetFanForm::showSeriesByMode()
{
    switch (_dataFan->data().config.fanMode)
    {
    case FANMODE_LINEAR:
        indexSensorLinear = _dataFan->data().config.snsIdx;
        _dataSensor = _dataSensors[indexSensorLinear];
        _seriesFanSensor->show();
        _seriesFanSetpoint->hide();
        break;
    case FANMODE_LINEAR_TRIP_POINTS:
    case FANMODE_TRIP_POINTS:
        indexSensorTripPoints = _dataFan->data().config.snsIdx;
        _dataSensor = _dataSensors[indexSensorTripPoints];
        _seriesFanSensor->show();
        _seriesFanSetpoint->hide();
        break;
    case FANMODE_PI:
        indexSensorPiController = _dataFan->data().config.snsIdx;
        _dataSensor = _dataSensors[indexSensorPiController];
        _seriesFanSensor->show();
        _seriesFanSetpoint->show();
        break;
    default:
        _seriesFanSensor->hide();
        _seriesFanSetpoint->hide();
        _dataSensor = _dataSensors.front();
        break;
    }

    if (_seriesFanSensor && _dataSensor)
        _seriesFanSensor->setName(_dataSensor->fullName());
}

void WidgetFanForm::on_dataUpdated()
{
    qDebug() << "on_dataUpdated " << _dataFan->name();

    setTitle(_dataFan);

    QSignalBlocker ct(ui->cbType);
    ui->cbType->setCurrentIndex(_typeToIndex[_dataFan->data().config.fanType]);

    QSignalBlocker cm(ui->cbMode);
    ui->cbMode->setCurrentIndex(_modeToIndex[_dataFan->data().config.fanMode]);


    QSignalBlocker spf(ui->spFixedDuty);
    ui->spFixedDuty->setValue(round(_dataFan->getScaledFixedDuty() * 100.0));

    QSignalBlocker hs(ui->hsFixedDuty);
    ui->hsFixedDuty->setValue(round(_dataFan->getScaledFixedDuty() * 100.0));

    QSignalBlocker ss(ui->spMinRpmStall);
    ui->spMinRpmStall->setValue(_dataFan->getMinRpmStalled());

    ui->spValueRpm->setValue(_dataFan->data().rpm);
    ui->spValueDuty->setValue(_dataFan->data().duty);

    updateSpinBox(ui->tbValue_1, _dataFan->data().trip_point[0].value);
    updateSpinBox(ui->tbValue_2, _dataFan->data().trip_point[1].value);
    updateSpinBox(ui->tbValue_3, _dataFan->data().trip_point[2].value);
    updateSpinBox(ui->tbValue_4, _dataFan->data().trip_point[3].value);
    updateSpinBox(ui->tbValue_5, _dataFan->data().trip_point[4].value);

    updateSpinBox(ui->tbDuty_1, _dataFan->data().trip_point[0].duty);
    updateSpinBox(ui->tbDuty_2, _dataFan->data().trip_point[1].duty);
    updateSpinBox(ui->tbDuty_3, _dataFan->data().trip_point[2].duty);
    updateSpinBox(ui->tbDuty_4, _dataFan->data().trip_point[3].duty);
    updateSpinBox(ui->tbDuty_5, _dataFan->data().trip_point[4].duty);

    updateDoubleSpinBox(ui->spPiKi, _dataFan->getPiControllerKi());
    updateDoubleSpinBox(ui->spPiKp, _dataFan->getPiControllerKp());
    updateDoubleSpinBox(ui->spPiKt, _dataFan->getPiControllerKt());

    updateDoubleSpinBox(ui->spPiSetpointOffset, _dataFan->getPiSetpointOffset());

    QSignalBlocker sl(ui->cbSensorLinear);
    QSignalBlocker sp(ui->cbSensorPiController);
    QSignalBlocker st(ui->cbSensorTripPoints);

    ui->cbSensorLinear->setCurrentIndex(indexSensorLinear);
    ui->cbSensorPiController->setCurrentIndex(indexSensorPiController);
    ui->cbSensorTripPoints->setCurrentIndex(indexSensorTripPoints);

    updateLinearControllerChart();
    updateTripPointControllerChart();

    showSeriesByMode();
}

void WidgetFanForm::appendLastValueToSeries(QLineSeries *series, QContiguousCache<TimeSeriesData> const &timeSeriesData)
{
    series->append(timeSeriesData.last().dt.toMSecsSinceEpoch(), timeSeriesData.last().value);
}

void WidgetFanForm::on_valueUpdated()
{
    ui->spValueRpm->setValue(_dataFan->data().rpm);
    ui->spValueDuty->setValue(_dataFan->data().duty);
    ui->statusFanStalled->setFanIsStalled(_dataFan->data().status.stalled);

    int sensorIndex = _dataFan->data().config.snsIdx;
    for (auto &sensor : _dataSensors)
    {
        if (sensorIndex == sensor->channel())
        {
            ui->spValueSensor->setValue(sensor->data().value);
            break;
        }
    }

    appendLastValueToSeries(_seriesFanRpm, _dataFan->seriesRpm());
    appendLastValueToSeries(_seriesFanDuty, _dataFan->seriesDuty());
    appendLastValueToSeries(_seriesFanSetpoint, _dataFan->seriesSetpoint());
    appendLastValueToSeries(_seriesFanSensor, _dataSensor->series());

    QDateTime now = QDateTime::currentDateTime();
    _axisXFan->setRange(now.addSecs(-60*60), now);
}

void WidgetFanForm::on_cbType_currentIndexChanged(int index)
{
    qDebug() << "on_cbType_currentIndexChanged " << index << " " << _dataFan->fullName();
    qDebug() << "on_cbType_currentIndexChanged " << ui->cbType->itemData(index).toInt() << " " << _dataFan->fullName();

    _dataFan->updateType((FANTYPE)ui->cbType->itemData(index).toInt());
    setTitle(_dataFan);
    //_chartFan->setTitle(_dataFan->fullName());

    qDebug() << "on_cbType_currentIndexChanged " << index << " " << _dataFan->data().config.fanType;
    qDebug() << "on_cbType_currentIndexChanged " << index << " " << _dataFan->fullName();
}

void WidgetFanForm::on_cbMode_currentIndexChanged(int index)
{
    qDebug() << "on_cbMode_currentIndexChanged " << index << " " << _dataFan->fullName();
    qDebug() << "on_cbMode_currentIndexChanged " << ui->cbMode->itemData(index).toInt() << " " << _dataFan->fullName();

    _dataFan->updateMode((FANMODE)ui->cbMode->itemData(index).toInt());
    setTitle(_dataFan);

    showSeriesByMode();

    qDebug() << "on_cbMode_currentIndexChanged " << index << " " << _dataFan->data().config.fanMode;
    qDebug() << "on_cbMode_currentIndexChanged " << index << " " << _dataFan->fullName();

    updateTripPointControllerChart();
}

void WidgetFanForm::on_spFixedDuty_valueChanged(int duty)
{
    _dataFan->setScaledFixedDuty(duty/100.0);
    _dataFan->updateFixedDuty(duty);
    QSignalBlocker b(ui->hsFixedDuty);
    ui->hsFixedDuty->setValue(duty);
}

void WidgetFanForm::on_hsFixedDuty_valueChanged(int duty)
{
    _dataFan->setScaledFixedDuty(duty/100.0);
    _dataFan->updateFixedDuty(duty);
    QSignalBlocker b(ui->spFixedDuty);
    ui->spFixedDuty->setValue(duty);
}

void WidgetFanForm::on_spMinRpmStall_valueChanged(int value)
{
    _dataFan->setMinRpmStalled(value);
    _dataFan->updateMinRpmStalled(value);
}

double WidgetFanForm::getLinearControllerPoint(double x1)
{
    return _dataFan->getLinearGain() * x1 + _dataFan->getLinearOffset();
}

void WidgetFanForm::on_spLinearGain_valueChanged(double value)
{
    _dataFan->updateLinearGain(value);
    updateLinearControllerChart();
}

void WidgetFanForm::on_spLinearOffset_valueChanged(double value)
{
    _dataFan->updateLinearOffset(value);
    updateLinearControllerChart();
}

void WidgetFanForm::updateTripPointValues()
{
    _dataFan->data().trip_point[0].value = ui->tbValue_1->value();
    _dataFan->data().trip_point[1].value = ui->tbValue_2->value();
    _dataFan->data().trip_point[2].value = ui->tbValue_3->value();
    _dataFan->data().trip_point[3].value = ui->tbValue_4->value();
    _dataFan->data().trip_point[4].value = ui->tbValue_5->value();

    _dataFan->updateTripPoints();

    updateTripPointControllerChart();
}

void WidgetFanForm::updateTripPointDutyCycleValues()
{
    _dataFan->data().trip_point[0].duty = ui->tbDuty_1->value();
    _dataFan->data().trip_point[1].duty = ui->tbDuty_2->value();
    _dataFan->data().trip_point[2].duty = ui->tbDuty_3->value();
    _dataFan->data().trip_point[3].duty = ui->tbDuty_4->value();
    _dataFan->data().trip_point[4].duty = ui->tbDuty_5->value();

    updateTripPointControllerChart();
}

void WidgetFanForm::updateTripPointSpinBoxes()
{
    ui->tbROValue_1->setValue(ui->tbValue_1->value());
    ui->tbROValue_2->setValue(ui->tbValue_2->value());
    ui->tbROValue_3->setValue(ui->tbValue_3->value());
    ui->tbROValue_4->setValue(ui->tbValue_4->value());
}

void WidgetFanForm::on_tbValue_1_editingFinished()
{
    if (ui->tbValue_2->value() < ui->tbValue_1->value())
        updateSpinBox(ui->tbValue_2, ui->tbValue_1->value());
    if (ui->tbValue_3->value() < ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_3, ui->tbValue_2->value());
    if (ui->tbValue_4->value() < ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_4, ui->tbValue_3->value());
    if (ui->tbValue_5->value() < ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_5, ui->tbValue_4->value());

    updateTripPointSpinBoxes();
    updateTripPointValues();
}

void WidgetFanForm::on_tbValue_2_editingFinished()
{
    if (ui->tbValue_1->value() > ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_1, ui->tbValue_2->value());

    if (ui->tbValue_3->value() < ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_3, ui->tbValue_2->value());
    if (ui->tbValue_4->value() < ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_4, ui->tbValue_3->value());
    if (ui->tbValue_5->value() < ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_5, ui->tbValue_4->value());

    updateTripPointSpinBoxes();
    updateTripPointValues();
}

void WidgetFanForm::on_tbValue_3_editingFinished()
{
    if (ui->tbValue_2->value() > ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_2, ui->tbValue_3->value());
    if (ui->tbValue_1->value() > ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_1, ui->tbValue_2->value());

    if (ui->tbValue_4->value() < ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_4, ui->tbValue_3->value());
    if (ui->tbValue_5->value() < ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_5, ui->tbValue_4->value());

    updateTripPointSpinBoxes();
    updateTripPointValues();
}

void WidgetFanForm::on_tbValue_4_editingFinished()
{
    if (ui->tbValue_3->value() > ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_3, ui->tbValue_4->value());
    if (ui->tbValue_2->value() > ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_2, ui->tbValue_3->value());
    if (ui->tbValue_1->value() > ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_1, ui->tbValue_2->value());

    if (ui->tbValue_5->value() < ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_5, ui->tbValue_4->value());

    updateTripPointSpinBoxes();
    updateTripPointValues();
}

void WidgetFanForm::on_tbValue_5_editingFinished()
{
    if (ui->tbValue_4->value() > ui->tbValue_5->value())
        updateSpinBox(ui->tbValue_4, ui->tbValue_5->value());
    if (ui->tbValue_3->value() > ui->tbValue_4->value())
        updateSpinBox(ui->tbValue_3, ui->tbValue_4->value());
    if (ui->tbValue_2->value() > ui->tbValue_3->value())
        updateSpinBox(ui->tbValue_2, ui->tbValue_3->value());
    if (ui->tbValue_1->value() > ui->tbValue_2->value())
        updateSpinBox(ui->tbValue_1, ui->tbValue_2->value());

    updateTripPointSpinBoxes();
    updateTripPointValues();
}

void WidgetFanForm::on_tbDuty_1_valueChanged(int value)
{
    Q_UNUSED(value);

    if (ui->tbDuty_2->value() < ui->tbDuty_1->value())
        updateSpinBox(ui->tbDuty_2, ui->tbDuty_1->value());
    if (ui->tbDuty_3->value() < ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_3, ui->tbDuty_2->value());
    if (ui->tbDuty_4->value() < ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_4, ui->tbDuty_3->value());
    if (ui->tbDuty_5->value() < ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_5, ui->tbDuty_4->value());

    updateTripPointDutyCycleValues();
}

void WidgetFanForm::on_tbDuty_1_editingFinished()
{
    on_tbDuty_1_valueChanged(ui->tbDuty_1->value());
    _dataFan->updateTripPoints();
}

void WidgetFanForm::on_tbDuty_2_valueChanged(int value)
{
    Q_UNUSED(value);

    if (ui->tbDuty_1->value() > ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_1, ui->tbDuty_2->value());

    if (ui->tbDuty_3->value() < ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_3, ui->tbDuty_2->value());
    if (ui->tbDuty_4->value() < ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_4, ui->tbDuty_3->value());
    if (ui->tbDuty_5->value() < ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_5, ui->tbDuty_4->value());

    updateTripPointDutyCycleValues();
}

void WidgetFanForm::on_tbDuty_2_editingFinished()
{
    on_tbDuty_2_valueChanged(ui->tbDuty_2->value());
    _dataFan->updateTripPoints();
}

void WidgetFanForm::on_tbDuty_3_valueChanged(int value)
{
    Q_UNUSED(value);

    if (ui->tbDuty_2->value() > ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_2, ui->tbDuty_3->value());
    if (ui->tbDuty_1->value() > ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_1, ui->tbDuty_2->value());

    if (ui->tbDuty_4->value() < ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_4, ui->tbDuty_3->value());
    if (ui->tbDuty_5->value() < ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_5, ui->tbDuty_4->value());

    updateTripPointDutyCycleValues();
}

void WidgetFanForm::on_tbDuty_3_editingFinished()
{
    on_tbDuty_3_valueChanged(ui->tbDuty_3->value());
    _dataFan->updateTripPoints();
}

void WidgetFanForm::on_tbDuty_4_valueChanged(int value)
{
    Q_UNUSED(value);

    if (ui->tbDuty_3->value() > ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_3, ui->tbDuty_4->value());
    if (ui->tbDuty_2->value() > ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_2, ui->tbDuty_3->value());
    if (ui->tbDuty_1->value() > ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_1, ui->tbDuty_2->value());

    if (ui->tbDuty_5->value() < ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_5, ui->tbDuty_4->value());

    updateTripPointDutyCycleValues();
}

void WidgetFanForm::on_tbDuty_4_editingFinished()
{
    on_tbDuty_4_valueChanged(ui->tbDuty_4->value());
    _dataFan->updateTripPoints();
}

void WidgetFanForm::on_tbDuty_5_valueChanged(int value)
{
    Q_UNUSED(value);

    if (ui->tbDuty_4->value() > ui->tbDuty_5->value())
        updateSpinBox(ui->tbDuty_4, ui->tbDuty_5->value());
    if (ui->tbDuty_3->value() > ui->tbDuty_4->value())
        updateSpinBox(ui->tbDuty_3, ui->tbDuty_4->value());
    if (ui->tbDuty_2->value() > ui->tbDuty_3->value())
        updateSpinBox(ui->tbDuty_2, ui->tbDuty_3->value());
    if (ui->tbDuty_1->value() > ui->tbDuty_2->value())
        updateSpinBox(ui->tbDuty_1, ui->tbDuty_2->value());

    updateTripPointDutyCycleValues();
}

void WidgetFanForm::on_tbDuty_5_editingFinished()
{
    on_tbDuty_5_valueChanged(ui->tbDuty_5->value());
    _dataFan->updateTripPoints();
}

void WidgetFanForm::on_currentTabChanged(int index)
{
    //if (index == _dataFan->channel())
    {
        updateSensorSelectors();
    }
}

void WidgetFanForm::showFanTypes()
{
    int index = 0;
    QSignalBlocker st(ui->cbType);
    for (int i = FANTYPE_NONE; i < FANTYPE_MAX; i++)
    {
        QString typeString = QString("%1 - %2").arg(usbfaceFanTypeToString((FANTYPE)i, 0)).arg(usbfaceFanTypeToString((FANTYPE)i, 1)) ;
        if (!typeString.isEmpty())
        {
            ui->cbType->addItem(typeString, i);
            _typeToIndex[(FANTYPE)i] = index;
            index++;
        }
    }
}

void WidgetFanForm::addFanModeToComboBox(FANMODE fanmode, int index)
{
    ui->cbMode->addItem(usbfaceFanModeToString(fanmode), fanmode);
    _modeToIndex[fanmode] = index;
}

void WidgetFanForm::showFanModes(int supportedFunctions)
{
    int index = 0;

    QSignalBlocker sb(ui->cbMode);
    ui->cbMode->clear();
    _modeToIndex.clear();

    addFanModeToComboBox(FANMODE_FIXED_DUTY, index);

    if (supportedFunctions & SUPPORTED_FANMODE_LINEAR)
        addFanModeToComboBox(FANMODE_LINEAR, index++);

    if (supportedFunctions & SUPPORTED_FANMODE_TP)
        addFanModeToComboBox(FANMODE_TRIP_POINTS, index++);

    if (supportedFunctions & SUPPORTED_FANMODE_LIN_TP)
        addFanModeToComboBox(FANMODE_LINEAR_TRIP_POINTS, index++);

    if (supportedFunctions & SUPPORTED_FANMODE_PI)
        addFanModeToComboBox(FANMODE_PI, index++);

    if (supportedFunctions & SUPPORTED_FANMODE_FUZZY)
        addFanModeToComboBox(FANMODE_FUZZY, index++);
}

void WidgetFanForm::on_supportedFunctionsUpdated(int supportedFunctions)
{
    ui->tabWidget->setTabEnabled(1, supportedFunctions & SUPPORTED_FANMODE_LINEAR);
    ui->tabWidget->setTabEnabled(2, supportedFunctions & (SUPPORTED_FANMODE_LIN_TP | SUPPORTED_FANMODE_TP));
    ui->tabWidget->setTabEnabled(3, supportedFunctions & (SUPPORTED_FANMODE_PI));
    ui->tabWidget->setTabEnabled(4, supportedFunctions & (SUPPORTED_FANMODE_FUZZY));

    showFanModes(supportedFunctions);
}

void WidgetFanForm::updateSensorSelectors()
{
    int currentPiSetpointSensor = ui->cbPiSetpoint->currentIndex();

    QSignalBlocker sl(ui->cbSensorLinear);
    QSignalBlocker sp(ui->cbSensorPiController);
    QSignalBlocker st(ui->cbSensorTripPoints);
    QSignalBlocker ss(ui->cbPiSetpoint);

    ui->cbSensorLinear->clear();
    ui->cbSensorPiController->clear();
    ui->cbSensorTripPoints->clear();
    ui->cbPiSetpoint->clear();

    for (auto &sensor : _dataSensors)
    {
        ui->cbSensorLinear->insertItem(sensor->channel(), sensor->fullName());
        ui->cbSensorTripPoints->insertItem(sensor->channel(), sensor->fullName());
        ui->cbSensorPiController->insertItem(sensor->channel(), sensor->fullName());
        ui->cbPiSetpoint->insertItem(sensor->channel(), sensor->fullName());
    }

    ui->cbSensorLinear->setCurrentIndex(indexSensorLinear);
    ui->cbSensorPiController->setCurrentIndex(indexSensorPiController);
    ui->cbSensorTripPoints->setCurrentIndex(indexSensorTripPoints);
    ui->cbPiSetpoint->setCurrentIndex(currentPiSetpointSensor);

    if (_seriesFanSensor && _dataSensor)
        _seriesFanSensor->setName(_dataSensor->fullName());
}

void WidgetFanForm::on_spPiSetpointOffset_valueChanged(double arg1)
{
    _dataFan->updatePiControllerSetpoint(ui->cbSensorPiController->currentIndex(), arg1);
}

void WidgetFanForm::on_spPiKp_valueChanged(double arg1)
{
    _dataFan->updatePiControllerParameters(arg1, ui->spPiKi->value(), ui->spPiKt->value());
}

void WidgetFanForm::on_spPiKi_valueChanged(double arg1)
{
    _dataFan->updatePiControllerParameters(ui->spPiKp->value(), arg1, ui->spPiKt->value());
}

void WidgetFanForm::on_spPiKt_valueChanged(double arg1)
{
    _dataFan->updatePiControllerParameters(ui->spPiKp->value(), ui->spPiKi->value(), arg1);
}

void WidgetFanForm::on_spMinRpm_valueChanged(int arg1)
{
    _dataFan->updateMinDuty(arg1);
}

void WidgetFanForm::on_spMaxRpm_valueChanged(int arg1)
{
    _dataFan->updateMaxDuty(arg1);
}

void WidgetFanForm::on_cbSensorTripPoints_currentIndexChanged(int index)
{
    _dataFan->updateSensorIndex(index);
    indexSensorTripPoints = index;
    _dataSensor = _dataSensors[index];
    _seriesFanSensor->setName(_dataSensor->fullName());
}

void WidgetFanForm::on_cbSensorLinear_currentIndexChanged(int index)
{
    _dataFan->updateSensorIndex(index);
    indexSensorLinear = index;
    _dataSensor = _dataSensors[index];
    _seriesFanSensor->setName(_dataSensor->fullName());
}

void WidgetFanForm::on_cbSensorPiController_currentIndexChanged(int index)
{
    _dataFan->updateSensorIndex(index);
    indexSensorPiController = index;
    _dataSensor = _dataSensors[index];
    _seriesFanSensor->setName(_dataSensor->fullName());
}

void WidgetFanForm::on_cbPiSetpoint_currentIndexChanged(int index)
{
    _dataFan->updatePiControllerSetpoint(index, ui->spPiSetpointOffset->value());
}

void WidgetFanForm::on_leDescription_editingFinished()
{
    setTitle(_dataFan);
}

void WidgetFanForm::on_sbMinDutyCycle_editingFinished()
{

}
