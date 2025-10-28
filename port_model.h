#ifndef PORT_MODEL_H
#define PORT_MODEL_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStandardItemModel>

class SerialPortModel : public QStandardItemModel
{
public:
    explicit SerialPortModel(QObject *parent = nullptr) : QStandardItemModel(parent)
    {

    }
    ~SerialPortModel()
    {

    }

    void addSerialPort(const QSerialPortInfo &serialPortInfo)
    {
        QString portName=serialPortInfo.portName()+"-"+serialPortInfo.description();
        QStandardItem *item = new QStandardItem(portName);
        item->setData(QVariant::fromValue(serialPortInfo), Qt::UserRole); // Store QBluetoothDeviceInfo as user data
        appendRow(item);
    }

    QSerialPortInfo getSerialPortInfo(const QModelIndex &index) const
    {
        QVariant data = itemData(index).value(Qt::UserRole);
        return data.value<QSerialPortInfo>();
    }
    // QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    // {
    //     if (!index.isValid() /*|| index.row() >= this->rowCount()*/) {
    //         return QVariant();
    //     }

    //     QVariant data = itemData(index).value(Qt::UserRole);
    //     return data.value<QSerialPortInfo>();

    //     if (role == Qt::DisplayRole) {
    //         // Only return the characteristic UUID as string for display purposes
    //         return details.characteristicUuid.toString();
    //     } else if (role == Qt::UserRole) {
    //         // You can store additional data under Qt::UserRole for internal use
    //         return QVariant::fromValue(details);
    //     }

    //     return QVariant();
    // }
};

#endif // PORT_MODEL_H
