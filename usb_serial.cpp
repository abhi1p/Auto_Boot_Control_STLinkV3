#include "usb_serial.h"

USB_Serial::USB_Serial(QObject *parent)
    : QObject{parent}
{}

USB_Serial::~USB_Serial()
{
    if(m_serialPort.isOpen())
    {
        m_serialPort.close();
    }
}

void USB_Serial::getAllSerialPort()
{
    qInfo()<<"Running in thread: "<<QThread::currentThread();
    m_serialPorts=QSerialPortInfo::availablePorts();
    for(QSerialPortInfo &portInfo:this->m_serialPorts)
    {
        // qInfo()<< "Port:" << portInfo.portName() << "\n"
        //         << "Location:" << portInfo.systemLocation() << "\n"
        //         << "Description:" << portInfo.description() << "\n"
        //         << "Manufacturer:" << portInfo.manufacturer() << "\n"
        //         << "Serial number:" << portInfo.serialNumber() << "\n"
        //         << "VID:" << portInfo.vendorIdentifier() << "\n"
        //         << "PID:" << portInfo.productIdentifier() << "\n";

        // if(portInfo.hasProductIdentifier() && portInfo.hasVendorIdentifier())
        // {
        //     emit this->foundSerialPort(portInfo);
        // }
        if(!portInfo.serialNumber().isEmpty())
        {
            emit foundSerialPort(portInfo);
        }



    }
}

void USB_Serial::openSerialPort(bool toOpen, QString serialPort, QSerialPort::BaudRate baudRate, QSerialPort::Parity parity, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits)
{
    qInfo()<<"To open: "<<toOpen;
    if(toOpen /*&& (!m_serialPort.isOpen())*/)
    {
        // m_serialPort.
        // m_serialPort.setPort(serialPortInfo);
        m_serialPort.setPortName(serialPort);
        emit log(QString("Connecting to ")+serialPort);
        // m_serialPort.setBreakEnabled()
        if(!m_serialPort.open(QIODevice::ReadWrite))
        {
            qInfo()<<"Serial port open error: "<<this->m_serialPort.error();

            // emit serialPortOpenedSignal(false);
        }
        else
        {
            m_serialPort.setBaudRate(baudRate);
            m_serialPort.setParity(parity);
            m_serialPort.setDataBits(dataBits);
            m_serialPort.setStopBits(stopBits);
            m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
            // emit serialPortOpenedSignal(true);
            connect(&m_serialPort,&QSerialPort::readyRead,this,&USB_Serial::readyRead,Qt::QueuedConnection);
            connect(&m_serialPort,&QSerialPort::errorOccurred,this,&USB_Serial::serialPortError,Qt::QueuedConnection);
            connect(&m_serialPort,&QSerialPort::aboutToClose,this,&USB_Serial::aboutToClose,Qt::QueuedConnection);
            qInfo()<<"Serial port opened";
        }
    }
    // else if((!toOpen) && m_serialPort.isOpen())
    else
    {
        qInfo()<<"To close serial port";
        m_serialPort.disconnect();
        if(m_serialPort.isOpen())
        {

            this->m_serialPort.close();
        }

        // emit serialPortOpenedSignal(false);
    }
    emit serialPortOpenedSignal(m_serialPort.isOpen());
    // else
    // {

    // }

}

void USB_Serial::sendCommand(QByteArray cmd)
{
    m_serialPort.write(cmd);
}

void USB_Serial::closePort()
{
    QString portname=m_serialPort.portName();
    m_serialPort.close();
    if(!m_serialPort.isOpen())
    {
        qInfo()<<"Port closed called";
        // emit portClosed(portname);
        emit portClosed();
        emit serialPortOpenedSignal(m_serialPort.isOpen());
    }
    m_lastSavedPort=portname;
}

void USB_Serial::openPort()
{
    m_serialPort.open(QIODevice::ReadWrite);
    if(m_serialPort.isOpen())
    {
        emit portOpened(m_serialPort.portName());
    }
    emit serialPortOpenedSignal(m_serialPort.isOpen());
}


void USB_Serial::readyRead()
{
    // qInfo()<<"Data received";
#if 1
    while(m_serialPort.canReadLine())
    {
        QByteArray bytes = m_serialPort.readLine(MAX_STRING_LEN);
        // qInfo()<<"Bytes received: "<<bytes;
        QString line = QString(bytes);
        // qInfo()<<"Received at serial: "<<line;
        line = line.trimmed();
        if (line.isEmpty())
        {
            continue;
        }
        else;
        {
            // qInfo()<<"Command emit: "<<line;
            emit receivedCommand(line);
        }
    }
#else
    QByteArray bytes=m_serialPort.readLine(MAX_STRING_LEN);
    qInfo()<<"Bytes received: "<<bytes;
    QString line = QString(bytes);
    qInfo()<<"Received at seial: "<<line;
    line = line.trimmed();
    emit receivedCommand(line);
#endif
}

void USB_Serial::serialPortError(QSerialPort::SerialPortError error)
{
    qInfo()<<"Serial port error: "<<error;
    switch(error)
    {

        case QSerialPort::NoError:
        {

            break;
        }
        case QSerialPort::DeviceNotFoundError:
        {
            break;
        }
        case QSerialPort::PermissionError:
        {
            QString text="Port already in use";
            emit log(text);
            // emit serialPortOpenedSignal(false);
            emit serialPortOpenedSignal(m_serialPort.isOpen());
            break;
        }
        case QSerialPort::OpenError:
        {
            QString text="Port already in use";
            emit log(text);
            // emit serialPortOpenedSignal(false);
            emit serialPortOpenedSignal(m_serialPort.isOpen());
            break;
        }
        case QSerialPort::WriteError:
        {
            break;
        }
        case QSerialPort::ReadError:
        {
            break;
        }
        case QSerialPort::ResourceError:
        {
            QString text="Device disconnected";
            emit log(text);
            // emit serialPortOpenedSignal(false);
            if(m_serialPort.isOpen())
            {
                m_serialPort.close();
            }

            emit serialPortOpenedSignal(m_serialPort.isOpen());
            break;
        }
        case QSerialPort::UnsupportedOperationError:
        {
            break;
        }
        case QSerialPort::UnknownError:
        {
            break;
        }
        case QSerialPort::TimeoutError:
        {
            break;
        }
        case QSerialPort::NotOpenError:
        {
            qInfo()<<"serial port already open";
            emit serialPortOpenedSignal(m_serialPort.isOpen());
            break;
        }
    }

}

void USB_Serial::aboutToClose()
{
    qInfo()<<"Serial port about to close";
}

