#ifndef WIDGET_POWERMETER_FORM_H
#define WIDGET_POWERMETER_FORM_H

#include "timeseries_data.h"
#include <QtCore/QContiguousCache>
#include <QWidget>
#include <QtCharts/QChartView>
#include <memory>

namespace Ui {
class WidgetPowerMeterForm;
}

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
}

class DataPowerMeter;
class SeriesPowerMeter;

class WidgetPowerMeterForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetPowerMeterForm(std::shared_ptr<DataPowerMeter> dataPowerMeter, QWidget *parent = 0);
    ~WidgetPowerMeterForm();

public slots:
    void on_supportedFunctionsUpdated(int supportedFunctions);    
    void on_currentTabChanged(int index);

private slots:
    void on_dataUpdated();
    void on_valueUpdated();

private:
    void createChart();
    void appendLastValueToSeries(QT_CHARTS_NAMESPACE::QLineSeries *series, const QContiguousCache<TimeSeriesData> &timeSeriesData);

    Ui::WidgetPowerMeterForm *ui;
    std::shared_ptr<DataPowerMeter> _dataPowerMeter;
    std::shared_ptr<SeriesPowerMeter> _seriesPowerMeter;
};

#endif // WIDGET_POWERMETER_FORM_H
