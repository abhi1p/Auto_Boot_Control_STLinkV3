QT       += core gui serialport bluetooth printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 #console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += PROGRAM_NAME=\\\"AutoBootControl\\\"
TARGET=AutoBootControl
# Only define in release builds
CONFIG(release, debug|release) {
    DEFINES += QT_NO_INFO_OUTPUT
    DEFINES += QT_NO_DEBUG_OUTPUT
    DEFINES += QT_NO_CRITICAL_OUTPUT
    DEFINES += QT_NO_WARNING_OUTPUT

}
RC_FILE = windows_icon.rc
SOURCES += \
    STLink_Bridge/ErrLog.cpp \
    STLink_Bridge/bridge.cpp \
    STLink_Bridge/criticalsectionlock.cpp \
    STLink_Bridge/stlink_device.cpp \
    STLink_Bridge/stlink_interface.cpp \
    ble.cpp \
    boot_control.cpp \
    command_parser.cpp \
    dialog.cpp \
    gpio_bridge.cpp \
    instrument.cpp \
    main.cpp \
    mainwindow.cpp \
    port.cpp \
    qcpdocumentobject.cpp \
    qcustomplot.cpp \
    usb_serial.cpp

HEADERS += \
    PDF_Format.h \
    STLink_Bridge/ErrLog.h \
    STLink_Bridge/STLinkUSBDriver.h \
    STLink_Bridge/bridge.h \
    STLink_Bridge/criticalsectionlock.h \
    STLink_Bridge/platform_include.h \
    STLink_Bridge/stlink_device.h \
    STLink_Bridge/stlink_fw_api_bridge.h \
    STLink_Bridge/stlink_fw_api_common.h \
    STLink_Bridge/stlink_fw_const_bridge.h \
    STLink_Bridge/stlink_if_common.h \
    STLink_Bridge/stlink_interface.h \
    STLink_Bridge/stlink_type.h \
    ble.h \
    boot_control.h \
    command_parser.h \
    dialog.h \
    gpio_bridge.h \
    instrument.h \
    mainwindow.h \
    parameters.h \
    port.h \
    port_model.h \
    qcpdocumentobject.h \
    qcustomplot.h \
    usb_serial.h

FORMS += \
    dialog.ui \
    mainwindow.ui \
    port.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    STLink_Bridge/STLinkUSBDriver.dll \
    windows_icon.rc

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/' -lvisa64
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/' -lvisa64

INCLUDEPATH += 'C:/Program Files/IVI Foundation/VISA/Win64/Include'
DEPENDPATH += 'C:/Program Files/IVI Foundation/VISA/Win64/Include'

RESOURCES += \
    Icons.qrc
