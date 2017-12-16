#ifndef WIDGET_FASTLED_FORM_H
#define WIDGET_FASTLED_FORM_H

#include <QWidget>
#include <memory>
#include <list>

namespace Ui {
class WidgetFastLEDForm;
}

class DataFastLed;
class DataSensor;

class WidgetFastLEDForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFastLEDForm(std::shared_ptr<DataFastLed> dataFastLed, QWidget *parent = 0);
    ~WidgetFastLEDForm();

    void setDataSensors(std::vector<std::shared_ptr<DataSensor>> dataSensors);

public slots:
    void on_currentTabChanged(int index);
    void on_supportedFunctionsUpdated(int supportedFunctions);

private slots:
	void on_dataUpdated();
	void on_valueUpdated();

    void on_ColorSelector_colorChanged(const QColor &arg1);
    void on_ColorSelector_2_colorChanged(const QColor &arg1);

    void on_cbSensor_currentIndexChanged(int index);
    void on_checkBox_clicked(bool checked);
    void on_spinBox_valueChanged(int arg1);
    void on_cbAnimation_currentIndexChanged(int index);

private:
    void updateSensorSelectors();

    Ui::WidgetFastLEDForm *ui;

    std::shared_ptr<DataFastLed> _dataFastLed;
    std::shared_ptr<DataSensor> _dataSensor;
    std::vector<std::shared_ptr<DataSensor>> _dataSensors;
};

#endif // WIDGET_FASTLED_FORM_H
