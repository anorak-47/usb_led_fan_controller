#ifndef WIDGET_SENSOR_CONTAINER_FORM_H
#define WIDGET_SENSOR_CONTAINER_FORM_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCore/QDateTime>
#include <map>
#include <vector>

namespace Ui {
class WidgetSensorContainerForm;
}

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QChart;
class QDateTimeAxis;
class QValueAxis;
}

class WidgetSensorForm;
class QVBoxLayout;
class QSignalMapper;

class WidgetSensorContainerForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSensorContainerForm(QWidget *parent = 0);
    ~WidgetSensorContainerForm();

    void addWidgetSensor(WidgetSensorForm *sensor, bool hasSuccessor = true);

    void saveSettings();
    void readSettings();

public slots:
    void on_currentTabChanged(int index);

private slots:
    void on_sensorValueUpdated(QWidget *sensorWidget);
    void on_showGraphUpdated(QWidget *sensorWidget);
    void on_nameGraphUpdated(QWidget *sensorWidget);
    void on_colorGraphUpdated(QWidget *sensorWidget);

    void on_timeRangeChanged(QTime timerange);
    void on_pause(bool paused);

private:
    void restoreSplitter();
    void createSensorChart();
    void addSensorSeriesToGraph(WidgetSensorForm *sensorFormWidget);
    QString getNameForSeries(WidgetSensorForm *sensorFormWidget);

    Ui::WidgetSensorContainerForm *ui;

    int _time_range_secs = 60*60;
    bool _chart_paused = false;

    QVBoxLayout *_sensorLayout;
    QSignalMapper *_signalMapper;
    QSignalMapper *_signalMapperShowGraph;
    QSignalMapper *_signalMapperNameGraph;
    QSignalMapper *_signalMapperColorGraph;

    std::vector<WidgetSensorForm*> _widgetSensorForms;

    QT_CHARTS_NAMESPACE::QChart *_chart;
    std::map<QWidget*, QT_CHARTS_NAMESPACE::QLineSeries *> _lineSeries;

    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisX;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY;

    QT_CHARTS_NAMESPACE::QLineSeries *_firstLineSeries = 0;
};

#endif // WIDGET_SENSOR_CONTAINER_FORM_H
