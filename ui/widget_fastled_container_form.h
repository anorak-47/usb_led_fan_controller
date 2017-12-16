#ifndef WIDGET_FASTLED_CONTAINER_FORM_H
#define WIDGET_FASTLED_CONTAINER_FORM_H

#include <QWidget>

namespace Ui {
class WidgetFastLedContainerForm;
}

class WidgetFastLEDForm;
class QHBoxLayout;

class WidgetFastLedContainerForm : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetFastLedContainerForm(QWidget *parent = 0);
    ~WidgetFastLedContainerForm();

    void addWidgetFastLed(WidgetFastLEDForm *fastled);

private:
    Ui::WidgetFastLedContainerForm *ui;
    QHBoxLayout *_fastledLayout;
};

#endif // WIDGET_FASTLED_CONTAINER_FORM_H
