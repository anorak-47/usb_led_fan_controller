#ifndef WIDGET_FAN_H
#define WIDGET_FAN_H

#include "types.h"
#include "timeseries_data.h"
#include <QtCore/QContiguousCache>
#include <QtCharts/QChartView>
#include <QWidget>
#include <memory>
#include <list>
#include <map>

namespace Ui {
class WidgetFanForm;
}

class DataFan;
class SeriesFan;
class DataSensor;
class SeriesSensor;
class QSignalMapper;
class QSpinBox;
class QDoubleSpinBox;

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
}

class WidgetFanForm : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetFanForm(std::shared_ptr<DataFan> dataFan, QWidget *parent = nullptr);
    ~WidgetFanForm();

    void setDataSensors(std::vector<std::shared_ptr<DataSensor>> dataSensors);

signals:

public slots:
    void on_currentTabChanged(int index);
    void on_supportedFunctionsUpdated(int supportedFunctions);

private slots:
    void on_dataUpdated();
    void on_valueUpdated();

    void on_cbType_currentIndexChanged(int index);
    void on_cbMode_currentIndexChanged(int index);
    void on_spFixedDuty_valueChanged(int duty);
    void on_hsFixedDuty_valueChanged(int duty);
    void on_spMinRpmStall_valueChanged(int value);
    void on_spLinearGain_valueChanged(double value);
    void on_spLinearOffset_valueChanged(double value);

    void on_tbValue_1_editingFinished();
    void on_tbValue_2_editingFinished();
    void on_tbValue_3_editingFinished();
    void on_tbValue_4_editingFinished();
    void on_tbValue_5_editingFinished();

    void on_tbDuty_1_editingFinished();
    void on_tbDuty_2_editingFinished();
    void on_tbDuty_3_editingFinished();
    void on_tbDuty_4_editingFinished();
    void on_tbDuty_5_editingFinished();

    void on_tbDuty_1_valueChanged(int value);
    void on_tbDuty_2_valueChanged(int value);
    void on_tbDuty_3_valueChanged(int value);
    void on_tbDuty_4_valueChanged(int value);
    void on_tbDuty_5_valueChanged(int value);

    void on_spPiSetpointOffset_valueChanged(double arg1);
    void on_spPiKp_valueChanged(double arg1);
    void on_spPiKi_valueChanged(double arg1);
    void on_spPiKt_valueChanged(double arg1);
    void on_spMinRpm_valueChanged(int arg1);
    void on_spMaxRpm_valueChanged(int arg1);
    void on_cbSensorTripPoints_currentIndexChanged(int index);
    void on_cbSensorLinear_currentIndexChanged(int index);
    void on_cbSensorPiController_currentIndexChanged(int index);
    void on_cbPiSetpoint_currentIndexChanged(int index);

    void on_leDescription_editingFinished();

    void on_sbMinDutyCycle_editingFinished();

private:
    void updateSensorSelectors();

    void updateSpinBox(QSpinBox *box, int value);
    void updateDoubleSpinBox(QDoubleSpinBox *box, double value);

    void updateTripPointSpinBoxes();
    void updateTripPointValues();
    void updateTripPointDutyCycleValues();

    void addFanModeToComboBox(FANMODE fanmode, int index);
    void showFanModes(int supportedFunctions);

    void showFanTypes();
    double getLinearControllerPoint(double x1);

    void createLinearControllerChart();
    void updateLinearControllerChart();
    void createTripPointControllerChart();
    void updateTripPointControllerChart();

    void createFanChart();

    void appendLastValueToSeries(QT_CHARTS_NAMESPACE::QLineSeries *series, const QContiguousCache<TimeSeriesData> &timeSeriesData);

    void showSeriesByMode();
    void saveSettings();
    void readSettings();

    void setTitle(std::shared_ptr<DataFan> dataFan);

    Ui::WidgetFanForm *ui;
    std::shared_ptr<DataFan> _dataFan;
    std::shared_ptr<SeriesFan> _seriesFan;
    std::shared_ptr<DataSensor> _dataSensor;
    std::shared_ptr<SeriesSensor> _seriesSensor;
    std::vector<std::shared_ptr<DataSensor>> _dataSensors;

    int indexSensorPiController = 0;
    int indexSensorLinear = 0;
    int indexSensorTripPoints = 0;

    std::map<FANTYPE, int> _typeToIndex;
    std::map<FANMODE, int> _modeToIndex;

    QT_CHARTS_NAMESPACE::QChart *_chart;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesLinearController;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesTripPointController;
    void showSensorSeries();
};

#endif // WIDGET_FAN_H
