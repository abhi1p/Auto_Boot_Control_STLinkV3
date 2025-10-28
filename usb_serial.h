#ifndef USB_SERIAL_H
#define USB_SERIAL_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>

#define MAX_STRING_LEN (30)

class USB_Serial : public QObject
{
    Q_OBJECT
public:
    explicit USB_Serial(QObject *parent = nullptr);
    ~USB_Serial();

private:
    QSerialPort m_serialPort;
    QList<QSerialPortInfo> m_serialPorts;
    QString m_lastSavedPort;

signals:
    void foundSerialPort(QSerialPortInfo port);
    void serialPortOpenedSignal(bool opened);
    void receivedCommand(QString command);
    void log(QString text);
    void portClosed();
    // void portClosed(QString portname);
    void portOpened(QString portname);


public slots:
    void getAllSerialPort();
    // void selectSerialPort(QSerialPortInfo serialPortInfo, quint32 baudRate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits);
    // void openSerialPort(bool toOpen,QSerialPortInfo serialPortInfo=QSerialPortInfo(), quint32 baudRate=0,
    //                     QSerialPort::Parity parity=QSerialPort::OddParity, QSerialPort::DataBits dataBits=QSerialPort::Data8, QSerialPort::StopBits stopBits=QSerialPort::OneStop);
    void openSerialPort(bool toOpen,QString serialPort, QSerialPort::BaudRate baudRate,
                              QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits);

    void sendCommand(QByteArray cmd);

    void closePort();
    void openPort();

private slots:
    void readyRead();
    void serialPortError(QSerialPort::SerialPortError error);
    void aboutToClose();



signals:
};

#endif // USB_SERIAL_H
