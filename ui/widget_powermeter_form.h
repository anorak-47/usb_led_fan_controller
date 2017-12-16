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

class WidgetPowerMeterForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetPowerMeterForm(std::shared_ptr<DataPowerMeter> dataPowerMeter, QWidget *parent = 0);
    ~WidgetPowerMeterForm();

public slots:
    void on_supportedFunctionsUpdated(int supportedFunctions);

private slots:
    void on_dataUpdated();
    void on_valueUpdated();

private:
    void createChart();
    void appendLastValueToSeries(QT_CHARTS_NAMESPACE::QLineSeries *series, const QContiguousCache<TimeSeriesData> &timeSeriesData);

    Ui::WidgetPowerMeterForm *ui;
    std::shared_ptr<DataPowerMeter> _dataPowerMeter;

    QT_CHARTS_NAMESPACE::QChart *_chart;

    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisX;

    QT_CHARTS_NAMESPACE::QLineSeries *_seriesPower;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesCurrent;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesLoad;
};

#endif // WIDGET_POWERMETER_FORM_H