#-------------------------------------------------
#
# Project created by QtCreator 2017-11-29T08:20:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

TARGET = ulfcontroller
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../common $$PWD/../usb_raw $$PWD/../qt_custom_widgets

include($$PWD/../../Qt-Color-Widgets/color_widgets.pri)
include ($$PWD/../qt_custom_widgets/ulf_controller_customwidgets.pri)

unix|win32: LIBS += -lhidapi-libusb -lusb

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ../usb_raw/command_consumer.cpp \
    ../usb_raw/command_event.cpp \
    ../usb_raw/command_queue.cpp \
    ../usb_raw/command.cpp \
    ../usb_raw/connection_watcher.cpp \
    ../usb_raw/data_fan_out.cpp \
    ../usb_raw/data_fan.cpp \
    ../usb_raw/data_properties.cpp \
    ../usb_raw/data_sensor.cpp \
    ../usb_raw/data.cpp \
    ../usb_raw/timeseries_data.cpp \
    ../usb_raw/usb_connection.cpp \
    ../usb_raw/opendevice.c \
    widget_sensor_form.cpp \
    widget_fanout_form.cpp \
    widget_sensor_container_form.cpp \
    widget_fanout_container_form.cpp \
    widget_fan_form.cpp \
    widget_fastled_form.cpp \
    widget_deviceinformation_form.cpp \
    ../usb_raw/data_fastled.cpp \
    widget_fastled_container_form.cpp \
    ../usb_raw/value_updater.cpp \
    ../usb_raw/data_powermeter.cpp \
    widget_powermeter_form.cpp \
    widget_powermeter_container_form.cpp \
    chart_settings_form.cpp \
    widget_fan_container_form.cpp \
    widget_fan_show_form.cpp \
    series_data.cpp \
    series_fan.cpp \
    series_fanout.cpp \
    series_powermeter.cpp \
    series_sensor.cpp \
    value_updater_test.cpp \
    ../usb_raw/hid_device.cpp \
    ../usb_raw/usbface.cpp

HEADERS += \
        mainwindow.h \
    ../usb_raw/command_consumer.h \
    ../usb_raw/command_event.h \
    ../usb_raw/command_queue.h \
    ../usb_raw/command.h \
    ../usb_raw/connection_watcher.h \
    ../usb_raw/data_fan_out.h \
    ../usb_raw/data_fan.h \
    ../usb_raw/data_properties.h \
    ../usb_raw/data_sensor.h \
    ../usb_raw/data.h \
    ../usb_raw/opendevice.h \
    ../usb_raw/timeseries_data.h \
    ../usb_raw/usb_connection.h \
    ../usb_raw/usbface.h \
    ../common/types.h \
    ../common/config.h \
    ../common/fan_out_type.h \
    ../common/fan_type.h \
    ../common/requests.h \
    ../common/sensor_type.h \
    widget_sensor_form.h \
    widget_fanout_form.h \
    widget_sensor_container_form.h \
    widget_fanout_container_form.h \
    widget_fan_form.h \
    widget_fastled_form.h \
    widget_deviceinformation_form.h \
    ../usb_raw/data_fastled.h \
    widget_fastled_container_form.h \
    ../usb_raw/value_updater.h \
    ../usb_raw/data_powermeter.h \
    widget_powermeter_form.h \
    widget_powermeter_container_form.h \
    chart_settings_form.h \
    widget_fan_container_form.h \
    widget_fan_show_form.h \
    series_fan.h \
    series_data.h \
    series_fanout.h \
    series_powermeter.h \
    series_sensor.h \
    value_updater_test.h \
    ../usb_raw/hid_device.h

FORMS += \
        mainwindow.ui \
    widget_fan_form.ui \
    widget_sensor_form.ui \
    widget_fanout_form.ui \
    widget_sensor_container_form.ui \
    widget_fanout_container_form.ui \
    widget_fastled_form.ui \
    widget_deviceinformation_form.ui \
    widget_fastled_container_form.ui \
    widget_powermeter_form.ui \
    widget_powermeter_container_form.ui \
    chart_settings_form.ui \
    widget_fan_container_form.ui \
    widget_fan_show_form.ui

RESOURCES += \
    resources.qrc


