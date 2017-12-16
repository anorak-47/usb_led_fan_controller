#include "fanstalledwidget.h"
#include "ui_fanstalledwidget.h"

FanStalledWidget::FanStalledWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FanStalledWidget)
{
    ui->setupUi(this);    
    setFanIsStalled(false);
}

FanStalledWidget::~FanStalledWidget()
{
    delete ui;
}

void FanStalledWidget::setFanName(const QString &fanName)
{
    ui->label->setText(fanName);
}

QString FanStalledWidget::fanName() const
{
    return ui->label->text();
}

void FanStalledWidget::setFanIsStalled(bool stalled)
{
    _stalled = stalled;

    QIcon pm;

    if (stalled)
    {        
        pm = QIcon(":/pixmaps/gFanStalled.svg");
    }
    else
    {
        pm = QIcon(":/pixmaps/gFanNotStalled.svg");
    }

    ui->pixmap->setPixmap(pm.pixmap(QSize(20, 20), QIcon::Normal));
}

bool FanStalledWidget::isFanStalled() const
{
    return _stalled;
}
