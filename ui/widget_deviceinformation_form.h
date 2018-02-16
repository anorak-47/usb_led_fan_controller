#ifndef WIDGET_DEVICEINFORMATION_FORM_H
#define WIDGET_DEVICEINFORMATION_FORM_H

#include "types.h"
#include <QWidget>
#include <memory>

namespace Ui {
class WidgetDeviceInformationForm;
}

class DataDeviceProperties;
class QCheckBox;

class WidgetDeviceInformationForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetDeviceInformationForm(std::shared_ptr<DataDeviceProperties> properties, QWidget *parent = 0);
    ~WidgetDeviceInformationForm();

public slots:
    void on_settingsClear();
    void on_settingsLoad();
    void on_settingsSave();

private slots:
    void on_dataUpdated();
    void on_commandFinished();
    void on_usbCommunicationFailed(int errorCode, QString const& errorMsg);

    void on_pbSettingsSave_clicked();
    void on_pbSettingsLoad_clicked();
    void on_pbSettingsClear_clicked();

    void on_leName_editingFinished();

private:
    void enableSettingsButtons(bool enabled);
    void showSupportedFunctions(int supportedFunctions);

    Ui::WidgetDeviceInformationForm *ui;
    std::shared_ptr<DataDeviceProperties> _properties;
};

#endif // WIDGET_DEVICEINFORMATION_FORM_H
