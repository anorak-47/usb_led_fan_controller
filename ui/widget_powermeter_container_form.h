#ifndef WIDGET_POWERMETER_CONTAINER_FORM_H
#define WIDGET_POWERMETER_CONTAINER_FORM_H

#include <QWidget>

namespace Ui {
class WidgetPowerMeterContainerForm;
}

class WidgetPowerMeterForm;
class QVBoxLayout;
class QHBoxLayout;

class WidgetPowerMeterContainerForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetPowerMeterContainerForm(QWidget *parent = 0);
    ~WidgetPowerMeterContainerForm();

    void addWidgetPowerMeter(WidgetPowerMeterForm *pm);

private:
    Ui::WidgetPowerMeterContainerForm *ui;
    QHBoxLayout *_powermeterLayout;
};

#endif // WIDGET_POWERMETER_CONTAINER_FORM_H
