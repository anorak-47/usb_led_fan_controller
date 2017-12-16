#include "widget_fastled_form.h"
#include "ui_widget_fastled_form.h"
#include "data_fastled.h"
#include "data_sensor.h"
#include <QtCore/QDebug>

WidgetFastLEDForm::WidgetFastLEDForm(std::shared_ptr<DataFastLed> dataFastLed, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFastLEDForm),
    _dataFastLed(dataFastLed)
{
    ui->setupUi(this);
    ui->lTitle->setText(dataFastLed->fullName());

    on_supportedFunctionsUpdated(SUPPORTED_NONE);

    connect(_dataFastLed.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataFastLed.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));
}

WidgetFastLEDForm::~WidgetFastLEDForm()
{
    delete ui;
}

void WidgetFastLEDForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
    updateSensorSelectors();
}

void WidgetFastLEDForm::on_currentTabChanged(int index)
{
    //if (index == _dataFan->channel())
    {
        updateSensorSelectors();
    }
}

void WidgetFastLEDForm::on_supportedFunctionsUpdated(int supportedFunctions)
{
    ui->gbFastLed->setEnabled((supportedFunctions & SUPPORTED_LED_FASTLED));
}

void WidgetFastLEDForm::updateSensorSelectors()
{
    int currentSensor = ui->cbSensor->currentIndex();
    if (currentSensor < 0)
        currentSensor = 0;

    QSignalBlocker sl(ui->cbSensor);
    ui->cbSensor->clear();

    for (auto &sensor : _dataSensors)
        ui->cbSensor->insertItem(sensor->channel(), sensor->fullName());

    ui->cbSensor->setCurrentIndex(currentSensor);
}

void WidgetFastLEDForm::on_ColorSelector_colorChanged(const QColor &arg1)
{
    if (arg1 == _dataFastLed->getColor1())
        return;
    ui->GradientSlider->setFirstColor(arg1);
    _dataFastLed->updateColors(arg1, _dataFastLed->getColor2());
}

void WidgetFastLEDForm::on_ColorSelector_2_colorChanged(const QColor &arg1)
{
    if (arg1 == _dataFastLed->getColor2())
        return;
    ui->GradientSlider->setLastColor(arg1);
    _dataFastLed->updateColors(_dataFastLed->getColor1(), arg1);
}

void WidgetFastLEDForm::on_cbSensor_currentIndexChanged(int index)
{
    _dataFastLed->updateSensorIndex(index);
}

void WidgetFastLEDForm::on_checkBox_clicked(bool checked)
{
    _dataFastLed->updateAutoStart(checked);
}

void WidgetFastLEDForm::on_spinBox_valueChanged(int arg1)
{
    _dataFastLed->updateFps(arg1);
}

void WidgetFastLEDForm::on_cbAnimation_currentIndexChanged(int index)
{
    _dataFastLed->updateAnimationId(index);
}

void WidgetFastLEDForm::on_dataUpdated()
{
    QSignalBlocker bc1(ui->ColorSelector);
    ui->ColorSelector->setColor(_dataFastLed->getColor1());
    QSignalBlocker bc2(ui->ColorSelector_2);
    ui->ColorSelector_2->setColor(_dataFastLed->getColor2());
    QSignalBlocker bca(ui->cbAnimation);
    ui->cbAnimation->setCurrentIndex(_dataFastLed->getAnimationId());
    QSignalBlocker bcs(ui->cbSensor);
    ui->cbSensor->setCurrentIndex(_dataFastLed->getSensorIndex());
    QSignalBlocker bcb(ui->checkBox);
    ui->checkBox->setChecked(_dataFastLed->isAutoStart());
    QSignalBlocker bsf(ui->spinBox);
    ui->spinBox->setValue(_dataFastLed->getFps());

    ui->GradientSlider->setFirstColor(_dataFastLed->getColor1());
    ui->GradientSlider->setLastColor(_dataFastLed->getColor2());
}

void WidgetFastLEDForm::on_valueUpdated()
{
    on_dataUpdated();
}
