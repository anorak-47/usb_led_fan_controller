#include "widget_fastled_container_form.h"
#include "ui_widget_fastled_container_form.h"
#include "widget_fastled_form.h"
#include <QtGui/QLayout>

WidgetFastLedContainerForm::WidgetFastLedContainerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFastLedContainerForm)
{
    ui->setupUi(this);

    _fastledLayout = new QHBoxLayout();
    setLayout(_fastledLayout);
}

WidgetFastLedContainerForm::~WidgetFastLedContainerForm()
{
    delete ui;
}

void WidgetFastLedContainerForm::addWidgetFastLed(WidgetFastLEDForm *fastled)
{
    _fastledLayout->addWidget(fastled);
}
