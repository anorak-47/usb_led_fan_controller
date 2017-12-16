#ifndef FEATURESUPPORTEDWIDGET_H
#define FEATURESUPPORTEDWIDGET_H

#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

namespace Ui {
class FeatureSupportedWidget;
}

//QDESIGNER_WIDGET_EXPORT

class FeatureSupportedWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString featureText READ featureText WRITE setFeatureText)
    Q_PROPERTY(bool featureSupported READ isFeatureSupported WRITE setFeatureIsSupported)

public:
    explicit FeatureSupportedWidget(QWidget *parent = 0);
    ~FeatureSupportedWidget();

    void setFeatureText(QString const &featureText);
    QString featureText() const;

    void setFeatureIsSupported(bool supported);
    bool isFeatureSupported() const;

private:
    bool _supported = false;
    Ui::FeatureSupportedWidget *ui;
};

#endif // FEATURESUPPORTEDWIDGET_H
