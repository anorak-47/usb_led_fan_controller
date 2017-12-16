#include "chart_settings_form.h"
#include "ui_chart_settings_form.h"

ChartSettingsForm::ChartSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartSettingsForm)
{
    ui->setupUi(this);
    ui->teTimeRange->setTime(QTime(1,0));
}

ChartSettingsForm::~ChartSettingsForm()
{
    delete ui;
}

void ChartSettingsForm::on_pbPause_toggled(bool checked)
{
    emit signalPause(checked);
}

void ChartSettingsForm::on_teTimeRange_editingFinished()
{
    emit signalTimeRangeChanged(ui->teTimeRange->time());
}
