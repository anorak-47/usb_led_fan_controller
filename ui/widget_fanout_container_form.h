#ifndef WIDGET_FANOUT_CONTAINER_FORM_H
#define WIDGET_FANOUT_CONTAINER_FORM_H

#include <QWidget>
#include <QtCharts/QChartView>

namespace Ui {
class WidgetFanoutContainerForm;
}

namespace QT_CHARTS_NAMESPACE {
class QLineSeries;
class QValueAxis;
class QDateTimeAxis;
}

class WidgetFanOutForm;
class QVBoxLayout;

class WidgetFanoutContainerForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFanoutContainerForm(QWidget *parent = 0);
    ~WidgetFanoutContainerForm();

    void addWidgetFanout(WidgetFanOutForm *fanout);

private:
    void createChart();

    Ui::WidgetFanoutContainerForm *ui;
    QVBoxLayout *_fanoutLayout;

    QT_CHARTS_NAMESPACE::QChart *_chartFan;
    QT_CHARTS_NAMESPACE::QDateTimeAxis *_axisXFan;

    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanRpm;
    QT_CHARTS_NAMESPACE::QLineSeries *_seriesFanDuty;
};

#endif // WIDGET_FANOUT_CONTAINER_FORM_H
