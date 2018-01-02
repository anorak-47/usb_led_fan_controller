#ifndef WIDGET_FAN_SHOW_FORM_H
#define WIDGET_FAN_SHOW_FORM_H

#include <QWidget>
#include <memory>

namespace Ui {
class WidgetFanShowForm;
}

class DataFan;
class SeriesFan;
class DataSensor;
class SeriesSensor;

class WidgetFanShowForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFanShowForm(std::shared_ptr<DataFan> dataFan, QWidget *parent = 0);
    ~WidgetFanShowForm();

    void setDataSensors(std::vector<std::shared_ptr<DataSensor>> dataSensors);

    bool showGraphRpm() const;
    bool showGraphDuty() const;
    bool showGraphSensor() const;

    QColor colorRpm() const;
    QColor colorDuty() const;
    QColor colorSensor() const;

    std::shared_ptr<DataFan> dataFan() const;
    std::shared_ptr<SeriesFan> seriesFan() const;
    std::shared_ptr<DataSensor> dataSensor() const;
    std::shared_ptr<SeriesSensor> seriesSensor() const;

signals:
    void signalGraphColorChanged();
    void signalShowGraphChanged();
    void signalValueUpdated();

public slots:
    void on_nameChanged(QString const& name);

private slots:
    void on_valueUpdated();

    void on_checkBox_clicked();
    void on_checkBox_2_clicked();
    void on_checkBox_3_clicked();

    void on_ColorSelector_1_colorChanged(const QColor &arg1);
    void on_ColorSelector_2_colorChanged(const QColor &arg1);
    void on_ColorSelector_3_colorChanged(const QColor &arg1);

private:
    void readSettings();
    void saveSettings();
    void showSensorSeries(int index);

    Ui::WidgetFanShowForm *ui;

    std::shared_ptr<DataFan> _dataFan;
    std::shared_ptr<SeriesFan> _seriesFan;
    std::shared_ptr<SeriesSensor> _seriesSensor;
    std::vector<std::shared_ptr<DataSensor>> _dataSensors;
};

#endif // WIDGET_FAN_SHOW_FORM_H
