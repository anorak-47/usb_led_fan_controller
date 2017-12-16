#ifndef WIDGET_SENSOR_FORM_H
#define WIDGET_SENSOR_FORM_H

#include "types.h"
#include <QWidget>
#include <memory>

namespace Ui {
class WidgetSensorForm;
}

class DataSensor;

class WidgetSensorForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSensorForm(std::shared_ptr<DataSensor> dataSensor, QWidget *parent = 0);
    ~WidgetSensorForm();

    std::shared_ptr<DataSensor> dataSensor() const;

    bool showInGraph() const;
    void setShowInGraph(bool showInGraph);

    QColor getGraphColor() const;
    QString getDescription() const;

    void readSettings();
    void saveSettings();

signals:
    void signalValueUpdated();
    void signalShowGraph(bool show);
    void signalShowGraphChanged();
    void signalGraphNameChanged();
    void signalGraphColorChanged();

public slots:
    void on_supportedFunctionsUpdated(int supportedFunctions);
    
private slots:
    void on_dataUpdated();
    void on_valueUpdated();

    void on_comboBox_currentIndexChanged(int index);
    void on_spinBox_valueChanged(int value);
    void on_bShowGraph_clicked();
    void on_ColorSelector_colorChanged(const QColor &arg1);

    void on_lineEdit_editingFinished();

private:
    void setValueReadOnlyByType();
    void createFanChart();

    bool _showInGraph = false;

    Ui::WidgetSensorForm *ui;
    std::shared_ptr<DataSensor> _dataSensor;

    std::map<SNSTYPE, int> _typeToIndex;
};

#endif // WIDGET_SENSOR_FORM_H
