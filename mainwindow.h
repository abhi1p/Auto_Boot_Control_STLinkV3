#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "usb_serial.h"
#include <QDebug>
#include "port_model.h"
#include "boot_control.h"
#include "QFileDialog"
#include "command_parser.h"
#include "port.h"
#include <QComboBox>
#include <QLabel>
#include "parameters.h"
#include <QSettings>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connectUSBSignalAndSlots(USB_Serial *usb_serial);
    void connectBootSignalAndSlots(Boot_Control *boot);

signals:
    void openSerialPortSignal(bool toOpen,QString serialPort=QString(), QSerialPort::BaudRate baudRate=QSerialPort::Baud115200,
                        QSerialPort::Parity parity=QSerialPort::OddParity, QSerialPort::DataBits dataBits=QSerialPort::Data8, QSerialPort::StopBits stopBits=QSerialPort::OneStop);

    void scanAllSerialPortsSignal();
    // void startAutoBoot(bool toStart, QString commandPort, QString firmwarePort=QString());
    // void startAutoBoot(bool toStart, QString serialPort);

    void startAutoBoot(bool toStart, Parameters_t &parameters);
    void updateBinDir(QString binDir);

    void enableDisableMCUSelectionSignal();

public slots:
    void foundSerialPort(QSerialPortInfo port);
    void serialPortOpened(bool opened);
    void receivedCommand(QString command);
    void log(QString text);
    void updateLog(QString text);
    void updateDeviceID(QString deviceID);

private slots:
    void openSerialPort();
    void scanAllSerialPorts();

    void enableTestCodeDump(int state);
    void enableSaveResults(int state);
    void enableSaveLog(int state);
    void enableStackDump(int state);
    void enableProgramID(int state);
    void enableSendCommands(int state);
    void commandLineEditTextChanged(const QString &text);
    void enableMainCode1Dump(int state);
    void enableMainCode2Dump(int state);
    void enableFUSUpdate(int state);
    void browseFUSFilePath();
    void browseStackFilePath();
    void browseTestFirmwareFilePath();
    void browseCommandsFilePath();
    void browseResultsFilePath();
    void browseMainFirmware1FilePath();
    void browseMainFirmware2FilePath();
    void parseCommandsBtnClicked();
    bool parseCommands(QString &location);
    void startAutoBootClicked();
    void mcuSelectComboboxSelectionChanged(const QString &text);
    void enableDisableMCUSelection();

    void dummyRunStateChanged(int state);
    void updateParameters(Parameters_t &parameters);
    void parametersEnableDisable(bool toEnable);

    void updateFUSStartAddress(QString filepath);
    void updateStackStartAddress(QString filepath);
    QString getFUSStartAddress(QString path);
    QString getStackStartAddress(QString path);
    QString extractFUSAddressFromHtml(const QString &html, const QString &binaryName);
    QString extractStackAddressFromHtml(const QString &html, const QString &binaryName);
    QString extractAddressForBinaryAndChip(const QString &html, const QString &binaryName, const QString &chipName);
    // void browse
    void testTimerTimeout();

private:
    Ui::MainWindow *ui;
    // Port *m_portWidget;
    QToolBar *m_toolbar;
    QLabel *m_MCUSelectLabel;
    QStringList m_MCUList={"STM32WB5xxG","STM32WB5xxY","STM32WB5xxE","STM32WB5xxC","STM32WB1x", "Other"};
    QComboBox *m_MCUSelectComboBox;
    bool m_portOpened=false;
    SerialPortModel *m_port_model;
    Command_Parser m_commandParser;

    QSerialPort::BaudRate m_baudRate=QSerialPort::Baud115200;
    QSerialPort::Parity m_parity=QSerialPort::OddParity;
    QSerialPort::DataBits m_dataBits=QSerialPort::Data8;
    QSerialPort::StopBits m_stopBits=QSerialPort::OneStop;

    bool m_dummyRun=false;
    // QFileDialog *m_fileDialog;
    bool m_testFirmwareFileSelected=false;
    bool m_stackFileSelected=false;
    bool m_programID=false;
    bool m_commandsFileSelected=false;
    bool m_mainFirmware1FileSelected=false;
    bool m_mainFirmware2FileSelected=false;
    bool m_fusFileSelected=false;

    QString m_fusRedirectPath="";
    QString m_wStackRedirectPath="";
    QString m_testFirmRedirectPath="";
    QString m_cmdFileRedirectPath="";
    QString m_mainFirm1RedirectPath="";
    QString m_mainFirm2RedirectPath="";
    QString m_resultRedirectPath="";
    bool m_bootStarted=false;
    QTimer m_testTimer;
    int m_time=0;

    Test_Header_t m_testHeaderInfo;
    QList<Test_Info_t> m_testsInfo;

    bool m_saveLog=false;
    /// `QWidget::closeEvent` handler
    void closeEvent(QCloseEvent * event);

    void saveAllSettings();
    void loadAllSettings();

};
#endif // MAINWINDOW_H
