QT          += widgets uiplugin

QTDIR_build {
# This is only for the Qt build. Do not use externally. We mean it.
PLUGIN_TYPE = designer
PLUGIN_CLASS_NAME = ULFControllerWidgetPlugins
load(qt_plugin)
CONFIG += install_ok
} else {
# Public example:

CONFIG      += plugin
TEMPLATE    = lib

TARGET = $$qtLibraryTarget($$TARGET)

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

}

HEADERS     = \
    featuresupportedwidgetplugin.h \
    featuresupportedwidget.h \
    fanstalledwidget.h \
    fanstalledwidgetplugin.h \
    customwidgetplugin.h
SOURCES     = \
    featuresupportedwidgetplugin.cpp \
    featuresupportedwidget.cpp \
    fanstalledwidget.cpp \
    fanstalledwidgetplugin.cpp \
    customwidgetplugin.cpp

FORMS += \
    featuresupportedwidget.ui \
    fanstalledwidget.ui

RESOURCES += \
    customwidgetplugin.qrc

DISTFILES += \
    ulf_controller_customwidgets.pri
