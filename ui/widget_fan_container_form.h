#ifndef WIDGET_FAN_CONTAINER_FORM_H
#define WIDGET_FAN_CONTAINER_FORM_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <memory>
#include <vector>

namespace Ui {
class WidgetFanContainerForm;
}

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
}

class QSignalMapper;
class WidgetFanShowForm;
class QVBoxLayout;

class WidgetFanContainerForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFanContainerForm(QWidget *parent = 0);
    ~WidgetFanContainerForm();

    void addWidgetFan(WidgetFanShowForm *fanForm, bool hasSuccessor);

public slots:
    void on_sensorValueUpdated(QWidget *fanWidget);
    void on_showGraphUpdated(QWidget *fanWidget);
    void on_colorGraphUpdated(QWidget *fanWidget);

private:
    void saveSettings();
    void readSettings();
    void restoreSplitter();
    void createFanChart();

    QSignalMapper *_signalMapperValueUpdated;
    QSignalMapper *_signalMapperShowGraph;
    QSignalMapper *_signalMapperColorGraph;

    Ui::WidgetFanContainerForm *ui;

    QVBoxLayout *_layout;
    std::vector<WidgetFanShowForm*> _widgetFanShowForms;

    QT_CHARTS_NAMESPACE::QChart *_chartFan;
    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisXFan;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_duty;
    QT_CHARTS_NAMESPACE::QValueAxis *_axisY_rpm;

    std::vector<QT_CHARTS_NAMESPACE::QLineSeries *> _seriesFanRpm;
    std::vector<QT_CHARTS_NAMESPACE::QLineSeries *> _seriesFanDuty;
    std::vector<QT_CHARTS_NAMESPACE::QLineSeries *> _seriesFanSensor;
    void toggleSeries(WidgetFanShowForm *fanShowForm, int channel);
};

#endif // WIDGET_FAN_CONTAINER_FORM_H
