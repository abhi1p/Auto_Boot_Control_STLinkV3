#ifndef BOOT_CONTROL_H
#define BOOT_CONTROL_H

#include <QObject>
#include "instrument.h"
#include "parameters.h"
#include "usb_serial.h"
#include <QStringList>
#include <QProcess>
#include "gpio_bridge.h"
#include <QTimer>
#include <QPdfWriter>
#include <QPageSize>
#include <QTextDocument>
#include <QTemporaryDir>
#include "ble.h"
#include "qcustomplot.h"
#include "qcpdocumentobject.h"
#include "dialog.h"

struct PlotData
{
    QVector<double> frequencyHz;
    QVector<double> gainDb;
    QVector<double> phaseDeg;
};
class CustomLogTicker : public QCPAxisTickerLog
{
public:
    QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override
    {
        if (tick < 1000)
            return QString::number(static_cast<int>(tick));
        else if (tick < 1e6)
            return QString::number(tick / 1000.0, 'f', 0) + "k";
        else
            return QString::number(tick / 1e6, 'f', 0) + "M";
    }
};

class Boot_Control : public QObject
{
    Q_OBJECT
public:
    explicit Boot_Control(QObject *parent = nullptr, QWidget *mainWindow=nullptr);
    ~Boot_Control();
    void connectUSBSignalAndSlots(USB_Serial *usb);

public slots:
    // void startAutoBoot(bool toStart, QString commandPort, QString firmwarePort);
    // void startAutoBoot(bool toStart, QString serialPort);
    void startAutoBoot(bool toStart, Parameters_t &parameters);
    void receivedCommand(QString command);
    // void portClosed(QString portName);
    void portClosed();
    void portOpened(QString portName);

    void updateBinDir(QString binDir);


signals:
    void sendCommandSignal(QByteArray cmd);
    void log(QString text);
    void updateLog(QString text);
    void closePort();
    void openPort();

    void autoBootExecuteSignal();
    void PCBConnectedSignal();
    void PCBDisconnectedSignal();
    void checkMCUBootloaderModeCompletedSignal();
    // void MCUInBootloaderModeSignal();
    void startFUSUpdateSignal();
    void startFUSSignal();
    void startFUSCommandCompletedSignal();
    void startwStackDumpSignal();
    void runWirelessStackSignal();
    void startTestFirmwareDumpSignal();
    void startMainFirmware1DumpSignal();
    void startMainFirmware2DumpSignal();
    void autoBootFinishedSignal();
    void startdetectPCBConnection();

    void userTerminationSignal();
    void FUSUpdateExecuteSignal();
    void FUSExecutionCompletedSignal();
    void wStackDumpExecuteSignal();
    void wStackDumpCompletedSignal();
    void runWirelessStackCompletedSignal();
    void testFirmwareDumpExecuteSignal();
    void testFirmwareDumpExecuteCompletedSignal();
    void startTestingSignal();
    void startTestingExecuteSignal();
    void startUserAppFinishedSignal();
    void pdfExportStartSignal();
    void mainFirmware1DumpExecuteSignal();
    void mainFirmware1DumpExecuteCompletedSignal();
    void mainFirmware2DumpExecuteSignal();
    void mainFirmware2DumpExecuteCompletedSignal();
    void checkDeviceIdSignal();
    void checkDeviceIdExecuteSignal();
    void checkDeviceIdExecuteCompletedSignal();
    void programDeviceIdSignal();
    void programDeviceIdExecuteSignal();
    void programDeviceIdExecuteCompletedSignal();
    void updateDeviceIDSignal(QString deviceID);

    void openSerialPort(bool toOpen,QString serialPort="", QSerialPort::BaudRate baudRate=QSerialPort::Baud115200,
                        QSerialPort::Parity parity=QSerialPort::EvenParity, QSerialPort::DataBits dataBits=QSerialPort::Data8, QSerialPort::StopBits stopBits=QSerialPort::OneStop);


private slots:

    void autoBootExecute();
    void putMCUInBootloaderMode();
    void exitMCUFromBootloaderMode();
    void checkMCUBootloaderMode();
    void checkMCUBootloaderModeCompleted();
    void MCUInBootloaderMode();
    void FUSUpdate();
    void startFUS();
    void startFUSCommandCompleted();
    void FUSUpdateExecute();
    void FUSExecutionCompleted();
    void wStackDump();
    void wStackDumpExecute();
    void wStackDumpCompleted();
    void runWirelessStack();
    void runWirelessStackCompleted();
    void testFirmwareDump();
    void testFirmwareDumpExecute();
    void testFirmwareDumpExecuteCompleted();
    void startTesting();
    void startUserApp(QString startAddress="0x08000000");
    void startUserAppFinished();
    void startTestingExecute();
    void comPortInit();
    void serialPortOpened(bool opened);
    void startTestDelay();
    void instrumentQueryLoop();
    ViStatus instrumentQuery(ViSession session,QString writeCommand,ViChar *buffer);
    void commandDelay();
    void commandTimeout();
    void bleScanTimeout();
    void pdfExportStart();
    // void extracted(const Test_Output_t &out);
    void pdfExportExecute();
    void createPDF(QString htmlContent, QString pdfFilePath);
    void createPDF2(QString htmlContent, QString pdfFilePath);
    void testGraphInsert(QTextEdit *textEdit);
    void insertGraphs(QTextEdit *textEdit, QMap<QString, PlotData> &plots, int width);
    void plotGraph();
    void readBlockData(ViSession session, QString writeCommand,QByteArray &data);
    void parseBlockDataFormat1(QByteArray &data, QString plotName, QMap<QString, PlotData> &plots);
    void plot(QString plotTitle, QVector<double> &freq, QVector<double> &gain, QVector<double> &phase);
    void mainFirmware1Dump();
    void mainFirmware1DumpExecute();
    void mainFirmware1DumpExecuteCompleted();
    void mainFirmware2Dump();
    void mainFirmware2DumpExecute();
    void mainFirmware2DumpExecuteCompleted();
    void checkDeviceId();
    void checkDeviceIdExecute();
    void checkDeviceIdExecuteCompleted();
    void programDeviceId();
    void programDeviceIdExecute();
    void programDeviceIdExecuteCompleted();
    void detectPCBConnection();
    void detectPCBDisconnection();
    void PCBConnected();
    void PCBDisconnected();
    void autoBootFinished();

    void sendCommand(QString cmd);
    void readyReadProcessError();
    void readyReadProcessOutput();
    void errorOccurred(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus = QProcess::NormalExit);

    void startProgrammer(QString port);

    void processCommands(QString text);

    void userTermination();
    QString getVersionFromHex(QString version);

    void bleScanFinished();
    void discoveredBLEdevice(const QBluetoothDeviceInfo &deviceInfo);


private:
    // uint8_t m_controllerAddress=0xAA;
    QStringList m_cmdlist;
    QProcess m_process;
    QString m_binDir;
    quint8 m_uartAddr=0xAA;
    bool m_samePort=false;
    QString m_serialPort;
    QString m_firmwarePort;
    QStringList m_detailsToCheck={"Chip ID","BootLoader protocol version","Device name",
                                    "Flash size","Device type","Device CPU"};
                                    // "Flash size","Device type","Device CPU","RSS state","Error"};
    QStringList m_deviceDetails;
    QMap<QString,QString> m_receivedDetails;
    QList< QStringList > m_commandsRes;
    QMap<QString,int> m_GPIOPinMap;
    int m_GPIOmsDelay=150; //ms
    int m_GPIODetectDelay=1000; //ms
    GPIO_Bridge m_GPIOBridge;
    quint16 m_PCBCount=0;
    bool m_bootModeCheck=false;
    QString m_currentMainTask="None";
    QString m_currentBackgroundTask="None";
    bool m_fusStart=false;

    bool m_fusUpdateEnabled=false;
    bool m_wStackDumpEnabled=false;
    bool m_testFirmwareDumpEnabled=false;
    bool m_mainFirmwareDumpEnabled=false;

    QTimer m_timer;
    QString m_uinqueIdAddress="0x1FFF7590";
    QString m_uinqueIdAddressSize="12";//bytes

    Parameters_t m_parameters;
    bool m_userTerminated=false;
    bool m_updateWriteProgressEnabled=false;
    bool m_updateReadProgressEnabled=false;

    const QString m_fusError1="Current FUS version is higher or equal to the uploaded FUS version.";

    int m_testStartDelay=2; //sec
    int m_tempCounter=0;
    int m_commandCounter=0;
    QPair<QString,QPair<QString,QString>> m_expectedCmdResponse={"None",{"None","None"}};
    PDF_t m_pdf;

    QString m_unprogrammedDeviceID="0x11111111";
    QTemporaryDir m_tempDir;
    QString m_tempFilePath="";
    int m_commandTimeout=20000;
    int m_bleScanTimeout=30000;

    BLE m_bleScanner;
    QPair<QString,bool> m_bleDeviceScan;

    int m_resetPulseCount=1;

    Instrument m_visa;
    QSharedPointer<QCustomPlot> m_customplot;
    QScopedPointer<QCPDocumentObject> m_plotObjectHandler;
    QScopedPointer<QTextEdit> m_textEdit;
    QSharedPointer<Dialog> m_dialog;
    ViSession m_defaultRM;
    QMap<QString,ViSession> m_instruments;
    QMap<QString,QString> m_localVariables;
    QMap<QString, PlotData> m_plots;
};

#endif // BOOT_CONTROL_H
