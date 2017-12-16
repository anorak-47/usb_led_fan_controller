#ifndef CUSTOMWIDGETSPLUGIN_H
#define CUSTOMWIDGETSPLUGIN_H

#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

class CustomWidgetsPlugin: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

    public:
        CustomWidgetsPlugin(QObject *parent = 0);

        QList<QDesignerCustomWidgetInterface*> customWidgets() const;

    private:
        QList<QDesignerCustomWidgetInterface*> widgets;
};

#endif
