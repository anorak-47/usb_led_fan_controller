#include "widget_fan_show_form.h"
#include "ui_widget_fan_show_form.h"
#include "data_fan.h"
#include "series_fan.h"
#include "data_sensor.h"
#include "series_sensor.h"
#include <QtCore/QSettings>
#include <QtCore/QDebug>

WidgetFanShowForm::WidgetFanShowForm(std::shared_ptr<DataFan> dataFan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFanShowForm),
    _dataFan(dataFan)
{
    ui->setupUi(this);    
    updateHeaderText();

    _seriesFan = std::shared_ptr<SeriesFan>(new SeriesFan(_dataFan));

    connect(dataFan.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));

    readSettings();
}

WidgetFanShowForm::~WidgetFanShowForm()
{
    saveSettings();
    delete ui;
}

void WidgetFanShowForm::updateHeaderText()
{
    ui->lName->setText(_dataFan->fullName());
}

void WidgetFanShowForm::showSensorSeries(int index)
{
    qDebug() << __PRETTY_FUNCTION__ << " sensor " << index;

    std::shared_ptr<DataSensor> dataSensor = _dataSensors[index];
    _seriesSensor = std::shared_ptr<SeriesSensor>(new SeriesSensor(dataSensor));
    _seriesSensor->copyFromTimeSeries(dataSensor->timeDataSeries());
    _seriesSensor->setSeriesVisible(true);
    _seriesSensor->setVisible(true);
}

void WidgetFanShowForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
    showSensorSeries(_dataFan->data().config.snsIdx);
}

void WidgetFanShowForm::on_SensorIndexChanged()
{
    showSensorSeries(_dataFan->data().config.snsIdx);
}

void WidgetFanShowForm::on_currentTabChanged(int index)
{
    Q_UNUSED(index);

    if (showGraphSensor() && isVisible())
        showSensorSeries(_dataFan->data().config.snsIdx);
}

bool WidgetFanShowForm::showGraphRpm() const
{
    return ui->checkBox->isChecked();
}

bool WidgetFanShowForm::showGraphDuty() const
{
    return ui->checkBox_2->isChecked();
}

bool WidgetFanShowForm::showGraphSensor() const
{
    return ui->checkBox_3->isChecked();
}

QColor WidgetFanShowForm::colorRpm() const
{
    return ui->ColorSelector_1->color();
}

QColor WidgetFanShowForm::colorDuty() const
{
    return ui->ColorSelector_2->color();
}

QColor WidgetFanShowForm::colorSensor() const
{
    return ui->ColorSelector_3->color();
}

void WidgetFanShowForm::on_nameChanged(const QString &name)
{
    updateHeaderText();
    ui->lDescription->setText(name);
}

void WidgetFanShowForm::on_valueUpdated()
{
    emit signalValueUpdated();
}

void WidgetFanShowForm::saveSettings()
{
    QSettings settings("Anorak", "ULFControl");
    settings.setValue(QString("Fan%1/show_rpm").arg(_dataFan->channel()), ui->checkBox->isChecked());
    settings.setValue(QString("Fan%1/show_duty").arg(_dataFan->channel()), ui->checkBox_2->isChecked());
    settings.setValue(QString("Fan%1/show_sensor").arg(_dataFan->channel()), ui->checkBox_3->isChecked());
    settings.setValue(QString("Fan%1/color_rpm").arg(_dataFan->channel()), ui->ColorSelector_1->color().name());
    settings.setValue(QString("Fan%1/color_duty").arg(_dataFan->channel()), ui->ColorSelector_2->color().name());
    settings.setValue(QString("Fan%1/color_sensor").arg(_dataFan->channel()), ui->ColorSelector_3->color().name());
}

std::shared_ptr<DataSensor> WidgetFanShowForm::dataSensor() const
{
    return _dataSensors[_dataFan->data().config.snsIdx];
}

std::shared_ptr<SeriesSensor> WidgetFanShowForm::seriesSensor() const
{
    return _seriesSensor;
}

std::shared_ptr<DataFan> WidgetFanShowForm::dataFan() const
{
    return _dataFan;
}

std::shared_ptr<SeriesFan> WidgetFanShowForm::seriesFan() const
{
    return _seriesFan;
}

void WidgetFanShowForm::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    ui->lDescription->setText(settings.value(QString("Fan%1/description").arg(_dataFan->channel())).toString());
    QSignalBlocker b1(ui->ColorSelector_1);
    ui->ColorSelector_1->setColor(settings.value(QString("Fan%1/color_rpm").arg(_dataFan->channel())).toString());
    QSignalBlocker b2(ui->ColorSelector_2);
    ui->ColorSelector_2->setColor(settings.value(QString("Fan%1/color_duty").arg(_dataFan->channel())).toString());
    QSignalBlocker b3(ui->ColorSelector_3);
    ui->ColorSelector_3->setColor(settings.value(QString("Fan%1/color_sensor").arg(_dataFan->channel())).toString());

    QSignalBlocker bc1(ui->checkBox);
    ui->checkBox->setChecked(settings.value(QString("Fan%1/show_rpm").arg(_dataFan->channel()), false).toBool());
    QSignalBlocker bc2(ui->checkBox_2);
    ui->checkBox_2->setChecked(settings.value(QString("Fan%1/show_duty").arg(_dataFan->channel()), false).toBool());
    QSignalBlocker bc3(ui->checkBox_3);
    ui->checkBox_3->setChecked(settings.value(QString("Fan%1/show_sensor").arg(_dataFan->channel()), false).toBool());
}

void WidgetFanShowForm::on_checkBox_clicked()
{
    emit signalShowGraphChanged();
}

void WidgetFanShowForm::on_checkBox_2_clicked()
{
    emit signalShowGraphChanged();
}

void WidgetFanShowForm::on_checkBox_3_clicked()
{
    emit signalShowGraphChanged();
}

void WidgetFanShowForm::on_ColorSelector_1_colorChanged(const QColor &arg1)
{
    Q_UNUSED(arg1);
    emit signalGraphColorChanged();
}

void WidgetFanShowForm::on_ColorSelector_2_colorChanged(const QColor &arg1)
{
    Q_UNUSED(arg1);
    emit signalGraphColorChanged();
}

void WidgetFanShowForm::on_ColorSelector_3_colorChanged(const QColor &arg1)
{
    Q_UNUSED(arg1);
    emit signalGraphColorChanged();
}
