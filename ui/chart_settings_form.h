#ifndef CHART_SETTINGS_FORM_H
#define CHART_SETTINGS_FORM_H

#include <QWidget>
#include <QtCore/QDateTime>

namespace Ui {
class ChartSettingsForm;
}

class ChartSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChartSettingsForm(QWidget *parent = 0);
    ~ChartSettingsForm();

signals:
    void signalTimeRangeChanged(QTime timerange);
    void signalPause(bool paused);

private slots:
    void on_pbPause_toggled(bool checked);
    void on_teTimeRange_editingFinished();

private:
    Ui::ChartSettingsForm *ui;
};

#endif // CHART_SETTINGS_FORM_H
