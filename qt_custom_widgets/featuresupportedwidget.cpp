#include "featuresupportedwidget.h"
#include "ui_featuresupportedwidget.h"

FeatureSupportedWidget::FeatureSupportedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FeatureSupportedWidget)
{
    ui->setupUi(this);    
    setFeatureIsSupported(false);
}

FeatureSupportedWidget::~FeatureSupportedWidget()
{
    delete ui;
}

void FeatureSupportedWidget::setFeatureText(const QString &featureText)
{
    ui->label->setText(featureText);
}

QString FeatureSupportedWidget::featureText() const
{
    return ui->label->text();
}

void FeatureSupportedWidget::setFeatureIsSupported(bool supported)
{
    _supported = supported;

    QIcon pm;

    if (supported)
    {        
        pm = QIcon(":/pixmaps/005-checked.svg");
    }
    else
    {
        pm = QIcon(":/pixmaps/004-cancel-1.svg");
    }

    ui->pixmap->setPixmap(pm.pixmap(QSize(20, 20), QIcon::Normal));
}

bool FeatureSupportedWidget::isFeatureSupported() const
{
    return _supported;
}
