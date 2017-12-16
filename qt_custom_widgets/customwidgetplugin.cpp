#include "customwidgetplugin.h"
#include "fanstalledwidgetplugin.h"
#include "featuresupportedwidgetplugin.h"

CustomWidgetsPlugin::CustomWidgetsPlugin( QObject *parent )
                        : QObject( parent )
{
    widgets.append( new FeatureSupportedWidgetPlugin( this ) );
    widgets.append( new FanStalledWidgetPlugin( this ) );
}

QList<QDesignerCustomWidgetInterface*> CustomWidgetsPlugin::customWidgets() const
{
    return widgets;
}
