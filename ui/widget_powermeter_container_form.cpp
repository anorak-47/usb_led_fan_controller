#include "widget_powermeter_container_form.h"
#include "ui_widget_powermeter_container_form.h"
#include "widget_powermeter_form.h"
#include <QtGui/QLayout>

WidgetPowerMeterContainerForm::WidgetPowerMeterContainerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetPowerMeterContainerForm)
{
    ui->setupUi(this);

    _powermeterLayout = new QHBoxLayout();
    ui->wPowerMeter->setLayout(_powermeterLayout);
}

WidgetPowerMeterContainerForm::~WidgetPowerMeterContainerForm()
{
    delete ui;
}

void WidgetPowerMeterContainerForm::addWidgetPowerMeter(WidgetPowerMeterForm *pm)
{
    _powermeterLayout->addWidget(pm);
}
