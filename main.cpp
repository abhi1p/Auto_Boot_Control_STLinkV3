#include "mainwindow.h"

#include <QApplication>
#include "usb_serial.h"
#include "boot_control.h"
#include "gpio_bridge.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    USB_Serial usb(&a);
    Boot_Control boot(&a,&w);
    // GPIO_Bridge bridge(&a);
    boot.connectUSBSignalAndSlots(&usb);
    w.connectUSBSignalAndSlots(&usb);
    w.connectBootSignalAndSlots(&boot);
    w.show();
    return a.exec();
}
