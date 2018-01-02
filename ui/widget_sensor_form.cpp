#include "widget_sensor_form.h"
#include "ui_widget_sensor_form.h"
#include "data_sensor.h"
#include "series_sensor.h"
#include "usbface.h"
#include <QtCore/QSettings>
#include <QtCore/QDebug>

WidgetSensorForm::WidgetSensorForm(std::shared_ptr<DataSensor> dataSensor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSensorForm),
    _dataSensor(dataSensor)
{
    ui->setupUi(this);
    ui->label->setText(QString::number(dataSensor->channel()+1));
    _seriesSensor = std::shared_ptr<SeriesSensor>(new SeriesSensor(_dataSensor));

    on_supportedFunctionsUpdated(SUPPORTED_NONE);
    setValueReadOnlyByType();

    connect(_dataSensor.get(), SIGNAL(signalChanged()), this, SLOT(on_dataUpdated()));
    connect(_dataSensor.get(), SIGNAL(signalValueChanged()), this, SLOT(on_valueUpdated()));

    readSettings();

    _seriesSensor->setVisible(_showInGraph);
    _seriesSensor->setColor(ui->ColorSelector->color());
}

WidgetSensorForm::~WidgetSensorForm()
{
    saveSettings();
    delete ui;
}

void WidgetSensorForm::on_supportedFunctionsUpdated(int supportedFunctions)
{
    int index = 0;

    QSignalBlocker cb(ui->comboBox);
    ui->comboBox->clear();

    for (int i = SNSTYPE_NONE; i < SNSTYPE_MAX; i++)
    {
        if (usbfaceSnsTypeIsSupportedByFunctions(supportedFunctions, (SNSTYPE)i))
        {
            QString typeString = usbfaceSensorTypeToString((SNSTYPE)i);
            if (!typeString.isEmpty())
            {
                //ui->comboBox->addItem(QString("%1 - %2").arg(i).arg(typeString), i);
                ui->comboBox->addItem(typeString, i);
                _typeToIndex[(SNSTYPE)i] = index;
                index++;
            }
        }
    }
}

void WidgetSensorForm::setValueReadOnlyByType()
{
    if (_dataSensor->data().type >= SNSTYPE_EXT0 && _dataSensor->data().type <= SNSTYPE_EXT3)
    {
        ui->spinBox->setReadOnly(false);
        ui->spinBox->setButtonSymbols(QSpinBox::ButtonSymbols::UpDownArrows);
    }
    else
    {
        ui->spinBox->setReadOnly(true);
        ui->spinBox->setButtonSymbols(QSpinBox::ButtonSymbols::NoButtons);
        QSignalBlocker sb(ui->spinBox);
        ui->spinBox->setValue(_dataSensor->data().value);
    }
}

bool WidgetSensorForm::showInGraph() const
{
    return _showInGraph;
}

void WidgetSensorForm::setShowInGraph(bool showInGraph)
{
    _showInGraph = showInGraph;
}

QColor WidgetSensorForm::getGraphColor() const
{
    return ui->ColorSelector->color();
}

QString WidgetSensorForm::getDescription() const
{
    return ui->lineEdit->text();
}

std::shared_ptr<DataSensor> WidgetSensorForm::dataSensor() const
{
    return _dataSensor;
}

std::shared_ptr<SeriesSensor> WidgetSensorForm::seriesSensor() const
{
    return _seriesSensor;
}

void WidgetSensorForm::on_dataUpdated()
{
    qDebug() << "on_dataUpdated " << _dataSensor->name();    
    qDebug() << "on_dataUpdated type " << _dataSensor->data().type;
    qDebug() << "on_dataUpdated index " << _typeToIndex[(SNSTYPE)_dataSensor->data().type];

    QSignalBlocker cb(ui->comboBox);
    ui->comboBox->setCurrentIndex(_typeToIndex[(SNSTYPE)_dataSensor->data().type]);

    QSignalBlocker sb(ui->spinBox);
    ui->spinBox->setValue(_dataSensor->data().value);

    setValueReadOnlyByType();
}

void WidgetSensorForm::on_valueUpdated()
{
    QSignalBlocker sb(ui->spinBox);
    ui->spinBox->setValue(_dataSensor->data().value);
}

void WidgetSensorForm::on_comboBox_currentIndexChanged(int index)
{
    QString oldFullName = _dataSensor->fullName();

    qDebug() << "on_comboBox_currentIndexChanged " << index << " " << _dataSensor->fullName();
    qDebug() << "on_comboBox_currentIndexChanged " << ui->comboBox->itemData(index).toInt() << " " << _dataSensor->fullName();
    _dataSensor->updateType((SNSTYPE)ui->comboBox->itemData(index).toInt());
    setValueReadOnlyByType();
    qDebug() << "on_comboBox_currentIndexChanged " << index << " " << _dataSensor->data().type;
    qDebug() << "on_comboBox_currentIndexChanged " << index << " " << _dataSensor->fullName();

    if (oldFullName == ui->lineEdit->text())
        ui->lineEdit->setText(_dataSensor->fullName());

    emit signalGraphNameChanged();
}

void WidgetSensorForm::on_spinBox_valueChanged(int value)
{
    if (_dataSensor->data().type >= SNSTYPE_EXT0 && _dataSensor->data().type <= SNSTYPE_EXT3)
    {
        _dataSensor->updateValue(value);
    }
}

void WidgetSensorForm::on_bShowGraph_clicked()
{
    qDebug() << "on_bShowGraph_clicked " << ui->bShowGraph->isChecked() << " " << _dataSensor->name();
    _showInGraph = ui->bShowGraph->isChecked();
    _seriesSensor->setVisible(_showInGraph);
    emit signalShowGraph(_showInGraph);
    emit signalShowGraphChanged();
}

void WidgetSensorForm::saveSettings()
{
    //qDebug() << "WidgetSensorForm::saveSettings";
    QSettings settings("Anorak", "ULFControl");
    settings.setValue(QString("Sensor%1/showInGraph").arg(_dataSensor->channel()), _showInGraph);
    settings.setValue(QString("Sensor%1/GraphColor").arg(_dataSensor->channel()), ui->ColorSelector->color().name());
    settings.setValue(QString("Sensor%1/Description").arg(_dataSensor->channel()), ui->lineEdit->text());
}

void WidgetSensorForm::readSettings()
{
    //qDebug() << "WidgetSensorForm::readSettings";
    QSettings settings("Anorak", "ULFControl");
    _showInGraph = settings.value(QString("Sensor%1/showInGraph").arg(_dataSensor->channel()), false).toBool();
    QColor color(settings.value(QString("Sensor%1/GraphColor").arg(_dataSensor->channel())).toString());
    QString description = settings.value(QString("Sensor%1/Description").arg(_dataSensor->channel()), _dataSensor->fullName()).toString();

    QSignalBlocker b(ui->bShowGraph);
    ui->bShowGraph->setChecked(_showInGraph);

    QSignalBlocker bl(ui->lineEdit);
    ui->lineEdit->setText(description);

    QSignalBlocker bc(ui->ColorSelector);
    ui->ColorSelector->setColor(color);
}

void WidgetSensorForm::on_ColorSelector_colorChanged(const QColor &arg1)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(arg1);
    _seriesSensor->setColor(arg1);
    emit signalGraphColorChanged();
}

void WidgetSensorForm::on_lineEdit_editingFinished()
{
    //_seriesSensor->setName(ui->lineEdit->text());
    emit signalGraphNameChanged();
}
