#ifndef FanStalledWidget_H
#define FanStalledWidget_H

#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

namespace Ui {
class FanStalledWidget;
}

//FEATURESUPPORTEDWIDGETPLUGIN_H

class FanStalledWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString fanName READ fanName WRITE setFanName)
    Q_PROPERTY(bool fanStalled READ isFanStalled WRITE setFanIsStalled)

public:
    explicit FanStalledWidget(QWidget *parent = 0);
    ~FanStalledWidget();

    void setFanName(QString const &fanName);
    QString fanName() const;

    void setFanIsStalled(bool stalled);
    bool isFanStalled() const;

private:
    bool _stalled = true;
    Ui::FanStalledWidget *ui;
};

#endif // FanStalledWidget_H
