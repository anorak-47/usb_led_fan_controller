#ifndef WIDGET_FANOUT_FORM_H
#define WIDGET_FANOUT_FORM_H

#include "types.h"
#include <QWidget>
#include <QtCharts/QChartView>
#include <memory>
#include <map>

namespace Ui {
class WidgetFanOutForm;
}

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
}

class QCheckBox;
class DataFanOut;
class DataFan;
class DataSensor;
class FanStalledWidget;

class WidgetFanOutForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFanOutForm(std::shared_ptr<DataFanOut> dataFanOut, QWidget *parent = 0);
    ~WidgetFanOutForm();

    void setDataFans(std::vector<std::shared_ptr<DataFan>> dataFans);
    void setDataSensors(std::vector<std::shared_ptr<DataSensor>> dataSensors);

public slots:
    void on_supportedFunctionsUpdated(int supportedFunctions);

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_dataUpdated();
    void on_valueUpdated();

private:
    void showStalledFans();
    void showDutyCycle();
    void showFanOutModes();
    void createChart();

    Ui::WidgetFanOutForm *ui;
    std::shared_ptr<DataFanOut> _dataFanOut;
    std::vector<std::shared_ptr<DataFan>> _dataFans;
    std::vector<std::shared_ptr<DataSensor>> _dataSensors;

    std::map<FANOUTMODE, int> _modeToIndex;
    std::vector<QCheckBox*> _stallDetectionFans;
    std::vector<FanStalledWidget*> _stalledFans;

    QT_CHARTS_NAMESPACE::QChart *_chartFan;
    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisXFan;

    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanRpm;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanDuty;
};

#endif // WIDGET_FANOUT_FORM_H
