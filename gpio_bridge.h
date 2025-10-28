#ifndef GPIO_BRIDGE_H
#define GPIO_BRIDGE_H

#include <QObject>
#include "STLink_Bridge/bridge.h"
#include <QSharedPointer>
#include <QDebug>
#include <QTimer>

#define MIN_VALID_VCC   (2.8f)
#define MAX_VALID_VCC   (4.0f)

class GPIO_Bridge : public QObject
{
    Q_OBJECT
public:
    explicit GPIO_Bridge(QObject *parent = nullptr);
    ~GPIO_Bridge();

signals:
    void connectedToBridge(bool connected);

public slots:
    bool GPIOBridgeInit(QMap<QString,int> pinMap);
    bool connectToBridge(QMap<QString,int> pinMap);
    Brg_StatusT GPIOInitTest();
    void readGPIO();
    Brg_GpioValT readPin(int pinNo);
    void readAllPin(Brg_GpioValT *value);
    void writePin(int pinNo, Brg_GpioValT value);
    bool validVcc();
    void log(Brg_StatusT status);

private slots:
    Brg_StatusT SelectSTLink(STLinkInterface* pStlinkIf, int* pFirstDevNotInUse);
    void DisconnectToBridge();
    Brg_StatusT Connect(int deviceNb);
    Brg_StatusT GPIO_Init(QMap<QString,int> pinMap);


private:
    QSharedPointer<Brg> m_pBrg;
    QSharedPointer<STLinkInterface> m_pStlinkIf;
    char m_serialNumber[SERIAL_NUM_STR_MAX_LEN];
    // QString m_dllPath="D:/Development/Qt/Learning/Qt_Core_Begineer/Auto_Boot_Control/STLink_Bridge/STLinkUSBDriver.dll";
    QString m_dllPath="D:\\Development\\Qt/Learning\\Qt_Core_Begineer\\Auto_Boot_Control\\STLink_Bridge\\";
    QTimer m_timer;

};

#endif // GPIO_BRIDGE_H
