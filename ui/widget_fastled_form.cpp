#include "widget_fastled_form.h"
#include "ui_widget_fastled_form.h"
#include "data_fastled.h"
#include "data_sensor.h"
#include <QtCore/QSettings>
#include <QtCore/QDebug>

std::vector<QString> animation_names {"Gradient", "Rotating Palette", "Fire2012"};
std::vector<std::vector<QString>> animation_options {{"<no options>"},
                                                     {"RainbowColors", "RainbowStripeColors", "RainbowStripeColors (blend)", "CloudColors", "PartyColors", "PurpleAndGreenPalette", "SetupTotallyRandomPalette"},
                                                     {"HeatColors", "black to red to yellow to white", "black to blue to aqua to white", "black to red to white", "new palette every time"}};

WidgetFastLEDForm::WidgetFastLEDForm(std::shared_ptr<DataFastLed> dataFastLed, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFastLEDForm),
    _dataFastLed(dataFastLed)
{
    ui->setupUi(this);
    ui->lTitle->setText(dataFastLed->fullName());

    on_supportedFunctionsUpdated(SUPPORTED_NONE);

    updateAnimationComboBox();
    updateOptionComboBox();

    connect(_dataFastLed.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataFastLed.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));

    readSettings();
}

WidgetFastLEDForm::~WidgetFastLEDForm()
{
    saveSettings();
    delete ui;
}

void WidgetFastLEDForm::saveSettings()
{
    QSettings settings("Anorak", "ULFControl");
    settings.setValue(QString("LedStripe%1/description").arg(_dataFastLed->channel()), ui->leName->text());
}

void WidgetFastLEDForm::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    ui->leName->setText(settings.value(QString("LedStripe%1/description").arg(_dataFastLed->channel()), _dataFastLed->fullName()).toString());
}

void WidgetFastLEDForm::setDataSensors(std::vector<std::shared_ptr<DataSensor> > dataSensors)
{
    _dataSensors = dataSensors;
    updateSensorSelectors();
}

void WidgetFastLEDForm::on_currentTabChanged(int index)
{
    Q_UNUSED(index);
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

void WidgetFastLEDForm::on_ColorSelector_1_colorChanged(const QColor &arg1)
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

void WidgetFastLEDForm::updateAnimationComboBox()
{
    QSignalBlocker bl(ui->cbAnimation);
    ui->cbAnimation->clear();
    for (auto name : animation_names)
    {
        ui->cbAnimation->addItem(name);
    }
}

void WidgetFastLEDForm::updateOptionComboBox()
{
    QSignalBlocker bl(ui->cbOption);
    ui->cbOption->clear();
    for (auto name : animation_options[ui->cbAnimation->currentIndex()])
    {
        ui->cbOption->addItem(name);
    }
    ui->cbOption->setCurrentIndex(_dataFastLed->getOption());
}

void WidgetFastLEDForm::on_cbAnimation_currentIndexChanged(int index)
{
    updateOptionComboBox();
    _dataFastLed->updateAnimationId(index);
    _dataFastLed->updateValues();
}

void WidgetFastLEDForm::on_cbOption_currentIndexChanged(int index)
{
    _dataFastLed->updateAnimationOption(index);
}

void WidgetFastLEDForm::on_pbPlay_clicked()
{
    _dataFastLed->updateRunning(true);
    updateRunningStateLineEdit();
}

void WidgetFastLEDForm::on_pbStop_clicked()
{
    _dataFastLed->updateRunning(false);
    updateRunningStateLineEdit();
}

void WidgetFastLEDForm::updateRunningStateLineEdit()
{
    if (_dataFastLed->isRunning())
        ui->lStatus->setText("running");
    else
        ui->lStatus->setText("stopped");
}

void WidgetFastLEDForm::on_dataUpdated()
{
    qDebug() << __PRETTY_FUNCTION__;

    qDebug() << "animation id: " << _dataFastLed->getAnimationId();
    qDebug() << "option: " << _dataFastLed->getOption();
    qDebug() << "fps: " << _dataFastLed->getFps();
    qDebug() << "running: " << _dataFastLed->isRunning();
    qDebug() << "autostart: " << _dataFastLed->isAutoStart();

    QSignalBlocker bc1(ui->ColorSelector_1);
    ui->ColorSelector_1->setColor(_dataFastLed->getColor1());
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

    updateRunningStateLineEdit();
    updateOptionComboBox();
}

void WidgetFastLEDForm::on_valueUpdated()
{
    on_dataUpdated();
}
