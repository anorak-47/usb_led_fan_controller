#include "widget_fan_show_form.h"
#include "ui_widget_fan_show_form.h"
#include "data_fan.h"
#include "data_sensor.h"
#include <QtCore/QSettings>
#include <QtCore/QDebug>

WidgetFanShowForm::WidgetFanShowForm(std::shared_ptr<DataFan> dataFan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFanShowForm),
    _dataFan(dataFan)
{
    ui->setupUi(this);    
    ui->lName->setText(_dataFan->fullName());
    readSettings();
}

WidgetFanShowForm::~WidgetFanShowForm()
{
    saveSettings();
    delete ui;
}

void WidgetFanShowForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
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
    ui->lDescription->setText(name);
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

std::shared_ptr<DataFan> WidgetFanShowForm::dataFan() const
{
    return _dataFan;
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
    qDebug() << __PRETTY_FUNCTION__;
    emit signalGraphColorChanged();
}

void WidgetFanShowForm::on_ColorSelector_2_colorChanged(const QColor &arg1)
{
    emit signalGraphColorChanged();
}

void WidgetFanShowForm::on_ColorSelector_3_colorChanged(const QColor &arg1)
{
    emit signalGraphColorChanged();
}