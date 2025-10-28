#include "boot_control.h"
#include "PDF_Format.h"

Boot_Control::Boot_Control(QObject *parent, QWidget *mainWindow)
    : QObject{parent},m_tempDir(QDir::tempPath())
{
    connect(&m_process,&QProcess::readyReadStandardError,this,&Boot_Control::readyReadProcessError,Qt::QueuedConnection);
    connect(&m_process,&QProcess::readyReadStandardOutput,this,&Boot_Control::readyReadProcessOutput,Qt::QueuedConnection);
    connect(&m_process,&QProcess::errorOccurred,this,&Boot_Control::errorOccurred,Qt::QueuedConnection);
    connect(&m_process,&QProcess::finished,this,&Boot_Control::finished,Qt::QueuedConnection);

    m_GPIOPinMap["Reset"]=0;
    m_GPIOPinMap["Boot"]=1;
    m_GPIOPinMap["Connect"]=2;
    m_GPIOPinMap["NC"]=3;
    // m_GPIOBridge.GPIOBridgeInit(m_GPIOPinMap);
    // qInfo()<<"MCU in bootloader mode: "<<mcuInBootloaderMode();

    connect(this,&Boot_Control::autoBootExecuteSignal,this,&Boot_Control::autoBootExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::PCBConnectedSignal,this,&Boot_Control::PCBConnected,Qt::QueuedConnection);
    connect(this,&Boot_Control::PCBDisconnectedSignal,this,&Boot_Control::PCBDisconnected,Qt::QueuedConnection);
    connect(this,&Boot_Control::checkMCUBootloaderModeCompletedSignal,this,&Boot_Control::checkMCUBootloaderModeCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::startFUSUpdateSignal,this,&Boot_Control::FUSUpdate,Qt::QueuedConnection);
    connect(this,&Boot_Control::startFUSSignal,this,&Boot_Control::startFUS,Qt::QueuedConnection);
    connect(this,&Boot_Control::startFUSCommandCompletedSignal,this,&Boot_Control::startFUSCommandCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::FUSUpdateExecuteSignal,this,&Boot_Control::FUSUpdateExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::FUSExecutionCompletedSignal,this,&Boot_Control::FUSExecutionCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::startwStackDumpSignal,this,&Boot_Control::wStackDump,Qt::QueuedConnection);
    connect(this,&Boot_Control::runWirelessStackSignal,this,&Boot_Control::runWirelessStack,Qt::QueuedConnection);
    connect(this,&Boot_Control::wStackDumpExecuteSignal,this,&Boot_Control::wStackDumpExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::wStackDumpCompletedSignal,this,&Boot_Control::wStackDumpCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::runWirelessStackCompletedSignal,this,&Boot_Control::runWirelessStackCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::startTestFirmwareDumpSignal,this,&Boot_Control::testFirmwareDump,Qt::QueuedConnection);
    connect(this,&Boot_Control::testFirmwareDumpExecuteSignal,this,&Boot_Control::testFirmwareDumpExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::testFirmwareDumpExecuteCompletedSignal,this,&Boot_Control::testFirmwareDumpExecuteCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::startTestingSignal,this,&Boot_Control::startTesting,Qt::QueuedConnection);
    connect(this,&Boot_Control::startTestingExecuteSignal,this,&Boot_Control::startTestingExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::startUserAppFinishedSignal,this,&Boot_Control::startUserAppFinished,Qt::QueuedConnection);
    connect(this,&Boot_Control::pdfExportStartSignal,this,&Boot_Control::pdfExportStart,Qt::QueuedConnection);
    connect(this,&Boot_Control::startMainFirmware1DumpSignal,this,&Boot_Control::mainFirmware1Dump,Qt::QueuedConnection);
    connect(this,&Boot_Control::mainFirmware1DumpExecuteSignal,this,&Boot_Control::mainFirmware1DumpExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::mainFirmware1DumpExecuteCompletedSignal,this,&Boot_Control::mainFirmware1DumpExecuteCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::startMainFirmware2DumpSignal,this,&Boot_Control::mainFirmware2Dump,Qt::QueuedConnection);
    connect(this,&Boot_Control::mainFirmware2DumpExecuteSignal,this,&Boot_Control::mainFirmware2DumpExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::mainFirmware2DumpExecuteCompletedSignal,this,&Boot_Control::mainFirmware2DumpExecuteCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::checkDeviceIdSignal,this,&Boot_Control::checkDeviceId,Qt::QueuedConnection);
    connect(this,&Boot_Control::checkDeviceIdExecuteSignal,this,&Boot_Control::checkDeviceIdExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::checkDeviceIdExecuteCompletedSignal,this,&Boot_Control::checkDeviceIdExecuteCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::programDeviceIdSignal,this,&Boot_Control::programDeviceId,Qt::QueuedConnection);
    connect(this,&Boot_Control::programDeviceIdExecuteSignal,this,&Boot_Control::programDeviceIdExecute,Qt::QueuedConnection);
    connect(this,&Boot_Control::programDeviceIdExecuteCompletedSignal,this,&Boot_Control::programDeviceIdExecuteCompleted,Qt::QueuedConnection);
    connect(this,&Boot_Control::autoBootFinishedSignal,this,&Boot_Control::autoBootFinished,Qt::QueuedConnection);
    connect(this,&Boot_Control::userTerminationSignal,this,&Boot_Control::userTermination,Qt::QueuedConnection);

    qInfo()<<"Temp dir: "<<m_tempDir.path();

    m_dialog.reset(new Dialog,&QObject::deleteLater);
    m_customplot.reset(m_dialog->plotInstance());
    m_plotObjectHandler.reset(new QCPDocumentObject(this));
    m_textEdit.reset(m_dialog->textEditInstance());
    m_textEdit->document()->documentLayout()->registerHandler(QCPDocumentObject::PlotTextFormat, m_plotObjectHandler.data());

    //initialize visa resource manager
    if(viOpenDefaultRM(&m_defaultRM) != VI_SUCCESS)
    {
        qInfo()<<"Failed to initialize visa resource manager";
    }
}

Boot_Control::~Boot_Control()
{

}

void Boot_Control::connectUSBSignalAndSlots(USB_Serial *usb)
{
    connect(usb,&USB_Serial::receivedCommand,this,&Boot_Control::receivedCommand,Qt::QueuedConnection);
    // connect(usb,&USB_Serial::portClosed,this,&Boot_Control::portClosed,Qt::QueuedConnection);
    connect(usb,&USB_Serial::serialPortOpenedSignal,this,&Boot_Control::serialPortOpened,Qt::QueuedConnection);
    connect(this,&Boot_Control::sendCommandSignal,usb,&USB_Serial::sendCommand,Qt::QueuedConnection);
    connect(this,&Boot_Control::closePort,usb,&USB_Serial::closePort,Qt::QueuedConnection);
    connect(this,&Boot_Control::openSerialPort,usb,&USB_Serial::openSerialPort,Qt::QueuedConnection);

}

// void Boot_Control::startAutoBoot(bool toStart, QString commandPort, QString firmwarePort)
// {
//     if(toStart)
//     {
//         m_serialPort=commandPort;
//         m_firmwarePort=firmwarePort;
//         if(commandPort==firmwarePort)
//         {
//             m_samePort=true;
//         }
//         else
//         {
//             m_samePort=false;
//         }
//         emit log(QString("Command port: ")+commandPort+QString(" Firmware port: ")+firmwarePort);
//         if(m_cmdlist.size()>0)
//         {
//             m_cmdlist.clear();
//         }
//         sendCommand("APP_STARTED\n");
//     }
// }

void Boot_Control::startAutoBoot(bool toStart, Parameters_t &parameters)
{

    if(toStart)
    {
        m_parameters=parameters;
        qInfo()<<"Dummy run enabled: "<<m_parameters.dummyRunEnabled;
        qInfo()<<"Serial port: "<<m_parameters.serialPort;
        m_stlinkBridgeConnected=m_GPIOBridge.GPIOBridgeInit(m_GPIOPinMap);
        m_PCBCount=1;
        m_serialPort=parameters.serialPort;
        m_pdf.header=m_parameters.testHeaderInfo;
        emit autoBootExecuteSignal();
        // emit startTestingSignal();
    }
    else
    {
        m_userTerminated=true;
    }
}
void Boot_Control::autoBootExecute()
{
    if(m_deviceDetails.size()>0)
    {
        m_deviceDetails.clear();
    }
    emit openSerialPort(false);
    if(m_stlinkBridgeConnected)
    {
        QString str=QString("Waiting for PCB ")+QString::number(m_PCBCount)+QString(" to be connected");
        emit log(str);
        // while(!m_GPIOBridge.validVcc()) // wait till VCC reads 3V
        // {
        //     QThread::msleep(200);
        // }

        // while(!m_GPIOBridge.readPin(m_GPIOPinMap["Connect"])) //wait till pcb is connected
        // {
        //     QThread::msleep(200);
        // }
        m_timer.setSingleShot(false);
        disconnect(&m_timer,&QTimer::timeout,0,0);
        connect(&m_timer,&QTimer::timeout,this,&Boot_Control::detectPCBConnection,Qt::QueuedConnection);
        m_timer.start(m_GPIODetectDelay);
    }
    else
    {
        checkMCUBootloaderMode();
    }

}
void Boot_Control::detectPCBConnection()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    qInfo()<<"Valid VCC: "<<m_GPIOBridge.validVcc();
    qInfo()<<"Connect pin: "<<m_GPIOBridge.readPin(m_GPIOPinMap.value("Connect"));
    if(m_GPIOBridge.validVcc())
    {
        // m_GPIOBridge.readGPIO();
        // QThread::msleep(200);
        qInfo()<<"Connect pin: "<<m_GPIOBridge.readPin(m_GPIOPinMap.value("Connect"));
        if(m_GPIOBridge.readPin(m_GPIOPinMap.value("Connect"))==GPIO_SET)
        {
            m_timer.stop();
            QString str= QString("PCB ")+QString::number(m_PCBCount)+QString(" connected");
            emit log(str);
            emit PCBConnectedSignal();
        }
        m_timer.stop();
        QString str= QString("PCB ")+QString::number(m_PCBCount)+QString(" connected");
        emit log(str);
        emit PCBConnectedSignal();

    }
}

void Boot_Control::detectPCBDisconnection()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(!m_GPIOBridge.validVcc())
    {
        if(!m_GPIOBridge.readPin(m_GPIOPinMap.value("Connect")))
        {
            m_timer.stop();
            emit PCBDisconnectedSignal();
        }

    }
}

void Boot_Control::PCBConnected()
{
    putMCUInBootloaderMode();
    checkMCUBootloaderMode();
}

void Boot_Control::PCBDisconnected()
{
    QString str= QString("PCB ")+QString::number(m_PCBCount)+QString(" disconnected");
    emit log(str);
    m_PCBCount++;
    emit autoBootExecuteSignal(); //restart for next PCB
}

void Boot_Control::putMCUInBootloaderMode()
{
    emit log("Putting MCU in bootloader mode");
    QThread::msleep(m_GPIOmsDelay);
    m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_SET); //reset high
    QThread::msleep(m_GPIOmsDelay);
    m_GPIOBridge.writePin(m_GPIOPinMap.value("Boot"),GPIO_SET); //Boot high
    QThread::msleep(m_GPIOmsDelay);
    m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_RESET); //reset low
    QThread::msleep(m_GPIOmsDelay);
    m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_SET); //reset high

}

void Boot_Control::exitMCUFromBootloaderMode()
{
    emit log("Exiting MCU from bootloader mode");
    for(int i=0;i<m_resetPulseCount;i++)
    {
        QThread::msleep(m_GPIOmsDelay);
        m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_SET); //reset high
        QThread::msleep(m_GPIOmsDelay);
        m_GPIOBridge.writePin(m_GPIOPinMap.value("Boot"),GPIO_RESET); //Boot low
        QThread::msleep(m_GPIOmsDelay);
        m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_RESET); //reset low
        QThread::msleep(m_GPIOmsDelay);
        m_GPIOBridge.writePin(m_GPIOPinMap.value("Reset"),GPIO_SET); //reset high

    }

    emit log("Exited MCU from bootloader mode");
}

void Boot_Control::checkMCUBootloaderMode()
{
    // m_serialPort="COM3";
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -r32 ")+m_uinqueIdAddress+QString(" ")+m_uinqueIdAddressSize;
    // m_bootModeCheck=true;
    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    m_currentBackgroundTask="Boot_Mode_Check";
    m_process.startCommand(cmd);
    //while(!m_processCompleted); //wait till process is complete


}

void Boot_Control::checkMCUBootloaderModeCompleted()
{

    if(m_process.exitCode()==0)
    {

        //MCU in bootloader mode
        emit log("MCU in bootloader mode\n");


        foreach (QString str, m_detailsToCheck)
        {
            if(m_receivedDetails.contains(str))
            {
                m_pdf.header[str]=m_receivedDetails.value(str);
                QString str2=str+QString(": ")+m_receivedDetails.value(str);
                emit log(str2);
            }

        }
        if(m_receivedDetails.contains(m_uinqueIdAddress))
        {
            m_pdf.header["MCU ID"]=m_receivedDetails.value(m_uinqueIdAddress);
            QString str2=QString("Unique Chip ID: ")+m_receivedDetails.value(m_uinqueIdAddress);
            emit log(str2);
        }
        emit log("");
        MCUInBootloaderMode();
    }
    else
    {
        if(m_stlinkBridgeConnected)
        {
            emit log("Failed to put MCU in bootloader mode");
        }
        else
        {
            emit log("MCU not in bootloader mode");
        }

        emit autoBootFinishedSignal(); //end the boot for now
    }
}

void Boot_Control::MCUInBootloaderMode()
{
    if(!m_parameters.dummyRunEnabled)
    {
        emit startFUSUpdateSignal();
    }
    else
    {
        emit autoBootFinishedSignal();
    }

}

void Boot_Control::FUSUpdate()
{
    m_currentMainTask="FUS_Update";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.fusParameter.enabled)
    {
        //m_fusStart=true;

        emit startFUSSignal();
    }
    else
    {
        emit log("FUS update skipped");
        emit startwStackDumpSignal();
    }
    // m_fusStart=true;
    //emit startFUSSignal();
}

void Boot_Control::startFUS()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    // QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
    //               QString(" br=115200 -startfus");
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -fusgetstate");

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Starting FUS");
    m_currentBackgroundTask="FUS_Start";
    m_process.startCommand(cmd);

}

void Boot_Control::startFUSCommandCompleted()
{
    if(m_process.exitCode()==0)
    {
        if(m_receivedDetails.contains("RSS state"))
        {
            if(m_receivedDetails.value("RSS state")=="FUS_IDLE")
            {
                //FUS started
                emit log("FUS started");
                if(m_currentMainTask=="FUS_Update")
                {
                    emit FUSUpdateExecuteSignal();
                }
                else if(m_currentMainTask=="WStack_dump")
                {
                    emit wStackDumpExecuteSignal();
                }

            }
            else
            {
                emit startFUSSignal();
            }
        }
    }
    else
    {
        emit startFUSSignal();
    }
}

void Boot_Control::FUSUpdateExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -fwupgrade ")+m_parameters.fusParameter.filePath
        + " " +m_parameters.fusParameter.startAddress + " firstinstall=0 startstack=0 -v";

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("updating FUS");
    m_currentBackgroundTask="FUS_Update";
    m_process.startCommand(cmd);
}

void Boot_Control::FUSExecutionCompleted()
{
    qInfo()<<"FUS update execution completed";
    if(m_process.exitCode()==0)
    {
        //FUS updated
        qInfo()<<"FUS updated";
        emit log("FUS updated successfully");
        emit startwStackDumpSignal();
    }
    else
    {
        if(m_receivedDetails.contains("Error"))
        {
            if(m_receivedDetails.value("Error")==m_fusError1)
            {
                QString cVersion=getVersionFromHex(m_receivedDetails.value("Current FUS Version"));
                QString uVersion=getVersionFromHex(m_receivedDetails.value("Uploaded FUS Version"));

                emit log("Current FUS version: "+cVersion);
                emit log("Uploaded FUS version: "+uVersion);
                emit log("FUS update skipped as uploaded version matches the current version");
                emit startwStackDumpSignal();
            }
            else
            {
                qInfo()<<"FUS update failed";
                emit log("FUS update failed");
                emit autoBootFinishedSignal(); //exit for now
            }
        }
        else
        {
            qInfo()<<"FUS update failed";
            emit log("FUS update failed");
            emit autoBootFinishedSignal(); //exit for now
        }

    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::wStackDump()
{
    m_currentMainTask="WStack_dump";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.wStackParameter.enabled)
    {
        emit startFUSSignal();
    }
    else
    {
        emit log("Wireless stack dump skipped");
        emit startTestFirmwareDumpSignal();
    }
}

void Boot_Control::wStackDumpExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -fwupgrade ")+m_parameters.wStackParameter.filePath
                  + " " +m_parameters.wStackParameter.startAddress + " firstinstall=0 startstack=0 -v";

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Dumping wireless stack");
    m_currentBackgroundTask="WStack_dump";
    m_process.startCommand(cmd);
}

void Boot_Control::wStackDumpCompleted()
{
    qInfo()<<"Stack dump execution completed";
    if(m_process.exitCode()==0)
    {
        qInfo()<<"Stack dumped";
        emit log("Wireless stack dumped successfully");
        // emit startTestFirmwareDumpSignal();
        emit runWirelessStackSignal();
    }
    else
    {
        qInfo()<<"Stack dump failed";
        emit log("Stack dump failed");
        emit autoBootFinishedSignal(); //exit for now
    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::runWirelessStack()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -startwirelessstack");

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Starting wireless stack");
    m_currentBackgroundTask="WStack_Start";
    m_process.startCommand(cmd);
}

void Boot_Control::runWirelessStackCompleted()
{
    if(m_process.exitCode()==0)
    {
        if(m_receivedDetails.contains("RSS state"))
        {
            qInfo()<<"Stack start status: "<<m_receivedDetails.value("RSS state");
            if((m_receivedDetails.value("RSS state")=="FUS_ERROR") && (m_receivedDetails.value("Error")=="FUS_NOT_RUNNING"))
            {
                //Wireless stack started
                emit log("Wireless stack started");
                if(m_currentMainTask=="WStack_dump")
                {
                    emit startTestFirmwareDumpSignal();
                }

            }
            else
            {
                emit runWirelessStackSignal();
            }
        }
    }
    else
    {
        emit runWirelessStackSignal();
    }
}

void Boot_Control::testFirmwareDump()
{
    m_currentMainTask="Test_firmware_dump";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.testFirmParameter.enabled)
    {
        emit testFirmwareDumpExecuteSignal();
    }
    else
    {
        emit log("Test firmware dump skipped");
        // emit startMainFirmware1DumpSignal();
        emit startTestingSignal();
    }
}

void Boot_Control::testFirmwareDumpExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -w ")+m_parameters.testFirmParameter.filePath
                  + " " +m_parameters.testFirmParameter.startAddress + " -v";

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Dumping test firmware");
    m_currentBackgroundTask="Test_firmware_dump";
    m_process.startCommand(cmd);
}

void Boot_Control::testFirmwareDumpExecuteCompleted()
{
    qInfo()<<"Test firmware dump execution completed";
    if(m_process.exitCode()==0)
    {
        qInfo()<<"Test firmware dumped";
        emit log("Test firmware dumped successfully");
        emit startTestingSignal();
        // emit startMainFirmware1DumpSignal();
    }
    else
    {
        qInfo()<<"Test firmware dump failed";
        emit log("Test firmware dump failed");
        emit autoBootFinishedSignal(); //exit for now
    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::startTesting()
{
    m_currentMainTask="Test_Commands";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.commandsParameter.enabled)
    {
        //exit from bootloader mode
        exitMCUFromBootloaderMode();
        startUserApp();
        m_resetPulseCount=1;
        // comPortInit();

    }
    else
    {
        emit log("MCU tests skipped");
        emit startMainFirmware1DumpSignal();
    }

}

void Boot_Control::startUserApp(QString startAddress)
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -s ")+startAddress;

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Verifying if user application started at "+startAddress);
    m_currentBackgroundTask="Start_User_App";
    m_process.startCommand(cmd);
}

void Boot_Control::startUserAppFinished()
{
    if(m_process.exitCode()==0)
    {
        emit log("User application was not started");
        emit log("Starting user application");
        emit log("User application started");
        m_resetPulseCount=2;
        exitMCUFromBootloaderMode();
    }
    else
    {
        emit log("User application already started");
    }
    if(m_currentMainTask=="Test_Commands")
    {
        comPortInit();
    }
}


void Boot_Control::comPortInit()
{

    emit openSerialPort(true,m_serialPort,QSerialPort::Baud115200,QSerialPort::EvenParity,QSerialPort::Data8,QSerialPort::OneStop);
}

void Boot_Control::serialPortOpened(bool opened)
{
    if(opened)
    {
        //start timer
        m_timer.disconnect();
        connect(&m_timer,&QTimer::timeout,this,&Boot_Control::startTestDelay,Qt::QueuedConnection);
        emit log("Starting test in "+QString::number(m_testStartDelay)+"sec");
        m_tempCounter=m_testStartDelay;
        m_timer.start(1000);
    }
    else
    {

    }
}

void Boot_Control::startTestDelay()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    m_tempCounter--;
    // if(m_tempCounter>0)
    // {
    //     emit updateLog("Starting test in "+QString::number(m_tempCounter)+"sec");
    // }
    emit updateLog("Starting test in "+QString::number(m_tempCounter)+"sec");
    if(m_tempCounter==0)
    {
        m_timer.stop();
        emit log("Test started"+QString("\n"));
        qInfo()<<"To start the test";
        m_commandCounter=0;
        // m_pdf={}
        // m_pdf.header=m_parameters.testHeaderInfo;
        // m_pdf.testsTable={};
        m_plots.clear();
        m_localVariables.clear();
        m_pdf.testsTable.clear();
        emit startTestingExecuteSignal();
    }

}

void Boot_Control::instrumentQueryLoop()
{
    if(m_userTerminated)
    {
        m_timer.stop();
        emit userTerminationSignal();
        return;
    }
    QString instr=m_parameters.testsInfo.at(m_commandCounter).firstParam;
    ViSession session=m_instruments.value(instr);
    QString scpiCmd=m_parameters.testsInfo.at(m_commandCounter).secondParam;
    unsigned int bit=m_parameters.testsInfo.at(m_commandCounter).thirdParam.toUInt();
    unsigned int expectedVal=m_parameters.testsInfo.at(m_commandCounter).fourthParam.toUInt();

    ViChar buffer[256] = {0};
    ViStatus status =instrumentQuery(session,scpiCmd,buffer);
    if(status<VI_SUCCESS)
    {
        return;
    }

    int val=QString::fromLatin1(buffer).toUInt();
    qInfo()<<"Instrument: "<<instr<<" Command: "<<scpiCmd<<" Bit: "<<bit<<" Current value: "<<val<<" Expected val: "<<expectedVal;
    if((val&(1<<bit))==expectedVal)
    {
        m_timer.stop();
        m_commandCounter++;
        emit startTestingExecuteSignal();
    }
}

ViStatus Boot_Control::instrumentQuery(ViSession session, QString writeCommand, ViChar *buffer)
{
    return viQueryf(session, "%s\n", "%t", writeCommand.toLatin1().constData(), buffer);
}

void Boot_Control::commandDelay()
{
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    m_tempCounter--;
    emit updateLog("Waiting for "+QString::number(m_tempCounter)+"sec");
    if(m_tempCounter==0)
    {
        m_timer.stop();
        qInfo()<<"Delay finished";
        m_commandCounter++;
        emit startTestingExecuteSignal();
    }
}

void Boot_Control::commandTimeout()
{
    emit log("Timeout receiving command");
    emit openSerialPort(false);
    emit autoBootFinishedSignal(); //exit for now
}

void Boot_Control::bleScanTimeout()
{
    emit log("Timeout waiting for BLE device to advertise");
    emit autoBootFinishedSignal(); //exit for now
}

void Boot_Control::pdfExportStart()
{
    if(m_parameters.resultParameter.enableResultsExport && m_parameters.commandsParameter.enabled)
    {
        pdfExportExecute();
        // emit startMainFirmware1DumpSignal();
        emit autoBootFinishedSignal();
    }
    else
    {
        // emit startMainFirmware1DumpSignal();
        emit autoBootFinishedSignal();
    }
}

void Boot_Control::pdfExportExecute()
{
    QString header= fheader;
    QString aim=m_pdf.header.value("Aim");
    header=header.arg(aim);

    // QString pName="IKshana Pulse Pro"; //product name
    QString pName=m_pdf.header.value("Product Name"); //product name
    header=header.arg(pName);

    // QString pId="0x851234"; //product id
    QString pId=m_pdf.header.value("Product ID");
    header=header.arg(pId);

    QString mcName=m_pdf.header.value("Device name");
    header=header.arg(mcName);

    QString mcId=m_pdf.header.value("MCU ID");
    header=header.arg(mcId);

    QString hVersion=m_pdf.header.value("Hardware Version");
    header=header.arg(hVersion);

    QString fVersion=m_pdf.header.value("Firmware Version");
    header=header.arg(fVersion);

    QString rows;
    for(const Test_Output_t &out:std::as_const(m_pdf.testsTable))
    {

        rows+=frow.arg(out.testDescription, out.status, out.testReqirement);
    }
    QString table=ftable.arg(rows);
    QString html=fmainHtml.arg(header,table);
    QString fileName="Test_Report_"+pId+"_"+QDateTime::currentDateTime().toString("dd_MM_yyyy_h_m_a")+".pdf";
    // QString fileName="Test_Report_"+pId+".pdf";
    QString filePath=m_parameters.resultParameter.resultFilePath+QDir::separator()+fileName;
    createPDF2(html,filePath);
    // emit log("PDF genereated: "+m_parameters.resultParameter.resultFilePath);
    emit log("PDF genereated: "+filePath);

}

void Boot_Control::createPDF(QString htmlContent, QString pdfFilePath)
{
    QPdfWriter writer(pdfFilePath);

    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    writer.setPageMargins(QMarginsF(5, 0, 5, 5));  // mm

    QTextEdit textEdit;
    // textEdit.in
    QTextDocument doc;
    // textEdit.setDocument(&doc);
    doc.setHtml(htmlContent);
    // doc.
    doc.setPageSize(writer.pageLayout().pageSize().size(QPageSize::Point));
    // wri
    // testGraphInsert(doc);
    // doc.print(&writer);
}

void Boot_Control::createPDF2(QString htmlContent, QString pdfFilePath)
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFilePath);
    QMargins pageMargins(0, 0, 0, 0);

    QPageLayout pageLayout;
    pageLayout.setMode(QPageLayout::FullPageMode);
    // pageLayout.
    pageLayout.setOrientation(QPageLayout::Portrait);
    pageLayout.setPageSize(QPageSize(QPageSize::A4));
    pageLayout.setUnits(QPageLayout::Millimeter);
    pageLayout.setMargins(QMarginsF(pageMargins));
    printer.setPageLayout(pageLayout);

    QSize paintSize=printer.pageLayout().paintRectPixels(printer.resolution()).size();
    m_textEdit->document()->setPageSize(paintSize);
    int width=paintSize.width();
    qInfo()<<"A4 size width: "<<width;
    m_textEdit->insertHtml(htmlContent);
    // testGraphInsert(m_textEdit.data());
    insertGraphs(m_textEdit.data(),m_plots,width);
    m_textEdit->document()->print(&printer);
    // m_dialog->exec();

}

void Boot_Control::testGraphInsert(QTextEdit *textEdit)
{
    plotGraph();
    // register the plot document object (only needed once, no matter how many plots will be in the QTextDocument):
    // QCPDocumentObject *plotObjectHandler = new QCPDocumentObject(this);
    // doc.documentLayout()->registerHandler(QCPDocumentObject::PlotTextFormat, plotObjectHandler);

    // QTextCursor cursor = textEdit.textCursor();

    // // insert the current plot at the cursor position. QCPDocumentObject::generatePlotFormat creates a
    // // vectorized snapshot of the passed plot (with the specified width and height) which gets inserted
    // // into the text document.
    // cursor.insertText(QString(QChar::ObjectReplacementCharacter), QCPDocumentObject::generatePlotFormat(m_customplot.data(), 10, 10));

    // textEdit.setTextCursor(cursor);
    // // delete plotObjectHandler;

    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    // cursor.insertBlock();
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), QCPDocumentObject::generatePlotFormat(m_customplot.data(), 600, 500));
    textEdit->setTextCursor(cursor);

}

void Boot_Control::insertGraphs(QTextEdit *textEdit, QMap<QString, PlotData> &plots, int width)
{
    // Insert the graph title
    QTextBlockFormat centerBlock;
    centerBlock.setAlignment(Qt::AlignHCenter);

    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(16);
    titleFormat.setFontWeight(QFont::Bold);

    int count=0;
    for (auto i = plots.cbegin(), end = plots.cend(); i != end; ++i)
    {

        QString header=i.key();
        qInfo()<<"Plot header: "<<header;
        PlotData plotData=i.value();
        count++;
        // QString name="Plot "+QString::number(count);
        plot(header,plotData.frequencyHz,plotData.gainDb,plotData.phaseDeg);
        // plot(name,plotData.frequencyHz,plotData.gainDb,plotData.phaseDeg);
        QTextCursor cursor = textEdit->textCursor();
        // cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
        cursor.insertBlock(centerBlock);
        cursor.insertText(header, titleFormat);
        cursor.insertBlock();  // Blank line
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), QCPDocumentObject::generatePlotFormat(m_customplot.data(),700,500));
        cursor.insertBlock();  // Blank line
        textEdit->setTextCursor(cursor);

        // //clear the title and legend row
        // if(m_customplot->plotLayout()->rowCount()>1)
        // {
        //     m_customplot->plotLayout()->removeAt(0);
        //     m_customplot->plotLayout()->removeAt(0);
        // }

        // m_dialog->exec();
    }
}
void Boot_Control::plotGraph()
{
    QVector<double> freq,gain,phase;

    m_visa.readData("USB0::0x2A8D::0x0396::CN65027288::INSTR",freq,gain,phase);
    plot("Test plot",freq,gain,phase);


}

void Boot_Control::readBlockData(ViSession session, QString writeCommand, QByteArray &data)
{
    ViUInt32 retCount;
    ViStatus status = viWrite(session, (ViBuf)writeCommand.toLatin1().constData(), writeCommand.length(), &retCount);

    // Read header first (IEEE block)
    char header[11] = {0}; // Max # header is 2 + 9 digits + 1 (safe side)
    viRead(session, (ViBuf)header, 2, &retCount); // e.g. "#4"

    int numDigits = header[1] - '0';
    viRead(session, (ViBuf)(header + 2), numDigits, &retCount); // Read size part
    header[2 + numDigits] = '\0';
    int byteCount = QString(header + 2).toInt();
    qInfo()<<"Data size: "<<byteCount;

    // Read the full binary data block
    QByteArray buffer(byteCount, 0);
    status = viRead(session, (ViBuf)buffer.data(), byteCount, &retCount);
    data=buffer;


}

void Boot_Control::parseBlockDataFormat1(QByteArray &data, QString plotName, QMap<QString, PlotData> &plots)
{
    // Split into lines and parse
    QVector<double> freq,gain,phase;
    PlotData plotData;
    QList<QByteArray> lines = data.split('\n');
    for (int i = 1; i < lines.size(); ++i)// Skip header
    {
        const QByteArray& line = lines.at(i).trimmed();
        if (line.isEmpty()) continue;

        QList<QByteArray> fields = line.split(',');
        if (fields.size() < 4) continue;
        bool ok;
        double f = fields.at(1).trimmed().toDouble(&ok);
        double g = fields.at(3).trimmed().toDouble(&ok);
        double p = fields.at(4).trimmed().toDouble(&ok);
        freq.append(f);
        gain.append(g);
        phase.append(p);
        // QVector<double> row;
        // for (const QByteArray& field : std::as_const(fields))
        // {
        //     bool ok;
        //     double val = field.trimmed().toDouble(&ok);
        //     row.append(ok ? val : NAN);
        // }
        //result.append(row);
    }
    plotData.frequencyHz=freq;
    plotData.gainDb=gain;
    plotData.phaseDeg=phase;
    plots.insert(plotName,plotData);
}

void Boot_Control::plot(QString plotTitle, QVector<double> &freq, QVector<double> &gain, QVector<double> &phase)
{
    m_customplot->clearGraphs();
    // m_customplot->clearPlottables();
    // m_customplot->clearItems();
    // m_customplot.reset(m_dialog->plotInstance());
    // m_customplot->plotLayout()->removeAt(0);
    // m_customplot->clearPlottables();
    m_customplot->addGraph(m_customplot->xAxis,m_customplot->yAxis);
    m_customplot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    m_customplot->graph(0)->setLineStyle(QCPGraph::lsLine);
    // m_cutomplot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue

    m_customplot->addGraph(m_customplot->xAxis,m_customplot->yAxis2);
    m_customplot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
    m_customplot->graph(0)->setLineStyle(QCPGraph::lsLine);
    m_customplot->graph(0)->setName("Gain");
    m_customplot->graph(1)->setName("Phase");
    // m_customplot->legend->setVisible(true);

    // // Move legend to top of the layout, above plot
    // QCPTextElement *title = new QCPTextElement(m_customplot.data(), plotTitle, QFont("sans", 12, QFont::Bold));
    // m_customplot->plotLayout()->insertRow(0);
    // m_customplot->plotLayout()->addElement(0, 0, title);

    // // Move legend below title
    // m_customplot->plotLayout()->insertRow(1);
    // m_customplot->plotLayout()->addElement(1, 0, m_customplot->legend);

    // // Set row stretch factors: [Title, Legend, Plot]
    // m_customplot->plotLayout()->setRowStretchFactor(0, 0.05);  // Title gets minimal height
    // m_customplot->plotLayout()->setRowStretchFactor(1, 0.025);  // Legend gets small space
    // m_customplot->plotLayout()->setRowStretchFactor(2, 1.0);  // Plot takes most space

    // Align and style
    m_customplot->legend->setVisible(true);
    m_customplot->legend->setBrush(QBrush(Qt::white));
    m_customplot->legend->setBorderPen(QPen(Qt::black));
    m_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
    m_customplot->legend->setWrap(1);

    m_customplot->xAxis->setVisible(true);
    m_customplot->yAxis->setVisible(true);
    m_customplot->yAxis2->setVisible(true);
    m_customplot->xAxis->setLabel("Frequency(Hz)");
    m_customplot->yAxis->setLabel("Gain(dB)");
    m_customplot->yAxis2->setLabel("Phase(degree)");
    m_customplot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<CustomLogTicker> logTicker(new CustomLogTicker);
    logTicker->setLogBase(10);
    logTicker->setSubTickCount(9);
    logTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    m_customplot->xAxis->setTicker(logTicker);
    m_customplot->xAxis->setRange(freq.first(), freq.last());

    double gMin=gain.at(0);
    double gMax=gain.at(0);
    double pMin=phase.at(0);
    double pMax=phase.at(0);
    for(int i=0;i<freq.size();i++)
    {
        double g=gain.at(i);
        if(g>gMax)
        {
            gMax=gain.at(i);
        }
        if(g<gMin)
        {
            gMin=gain.at(i);
        }
        double p=phase.at(i);
        if(p>pMax)
        {
            pMax=phase.at(i);
        }
        if(p<pMin)
        {
            pMin=phase.at(i);
        }
    }
    m_customplot->yAxis->setRange(gMin-2, gMax+2);
    m_customplot->yAxis2->setRange(pMin-10, pMax+10);
    m_customplot->graph(0)->setData(freq, gain);
    m_customplot->graph(1)->setData(freq, phase);

    m_customplot->setInteractions(QCP::iSelectPlottables);
    m_customplot->replot();
}
void Boot_Control::startTestingExecute()
{
    if(m_commandCounter==m_parameters.testsInfo.size())
    {
        // emit pdfExportStartSignal();
        emit openSerialPort(false);
        emit startMainFirmware1DumpSignal();
        m_timer.stop();

    }
    else
    {
        QString command=m_parameters.testsInfo.at(m_commandCounter).command;
        QString firstParam=m_parameters.testsInfo.at(m_commandCounter).firstParam;
        if(command=="TEST")
        {
            // commandVal+="\n";
            m_expectedCmdResponse.first="TEST";
            m_expectedCmdResponse.second.first="CMD_ACK";
            m_expectedCmdResponse.second.second="CMD_NACK";
            // QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
            emit log("Starting " +firstParam);
            m_timer.disconnect();
            connect(&m_timer,&QTimer::timeout,this,&Boot_Control::commandTimeout,Qt::QueuedConnection);
            m_timer.start(m_commandTimeout);
            sendCommand(firstParam);
        }
        else if(command=="WRITE")
        {
            sendCommand(firstParam);
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else if(command=="DELAY")
        {
            int delay=firstParam.toInt();
            if(delay>0)
            {
                m_tempCounter=delay;
                m_timer.disconnect();
                connect(&m_timer,&QTimer::timeout,this,&Boot_Control::commandDelay,Qt::QueuedConnection);
                emit log("Waiting for "+QString::number(m_tempCounter)+"sec");
                m_timer.start(1000);
            }
        }
        else if(command=="TESTBLE")
        {
            m_timer.disconnect();
            connect(&m_timer,&QTimer::timeout,this,&Boot_Control::commandTimeout,Qt::QueuedConnection);
            m_timer.start(m_commandTimeout);
            //verify if ble device is already advertising

            m_expectedCmdResponse.first="TESTBLE";
            m_expectedCmdResponse.second.first="CMD_ACK";
            m_expectedCmdResponse.second.second="CMD_NACK";
            // QString str=m_parameters.testsInfo.at(m_commandCounter).commandValue;
            emit log("Starting " +firstParam);
            sendCommand(firstParam);
        }
        else if(command=="ICNT")
        {
            //alias name=secondParameter
            QString aliasName=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            ViSession session=VI_NULL;
            ViStatus status = viOpen(m_defaultRM,(ViRsrc)firstParam.toLatin1().constData(),VI_NULL,VI_NULL,&session);
            if(status==VI_SUCCESS)
            {
                if(!m_instruments.contains(aliasName))
                {
                    m_instruments[aliasName]=session;
                }
                QString str="Connected to "+firstParam+ " as "+aliasName;
                emit log(str);

            }
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else if(command=="IDCNT")
        {
            //name=firstParameter
            if(m_instruments.contains(firstParam))
            {
                ViSession session=m_instruments.value(firstParam);
                viClose(session);
                m_instruments.remove(firstParam);
                QString str="Disconnected to "+firstParam;
                emit log(str);

            }
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else if(command=="IWRITE")
        {
            ViSession session=m_instruments.value(firstParam);
            QString scpiCommand=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            ViUInt32 retCount;
            ViStatus status = viWrite(session, (ViBuf)scpiCommand.toLatin1().constData(), scpiCommand.length(), &retCount);
            if(status==VI_SUCCESS)
            {
                QString str="Written "+scpiCommand+" to "+firstParam;
                emit log(str);

            }
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else if(command=="IREADS")
        {
            ViSession session=m_instruments.value(firstParam);
            QString variableName=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            ViChar buffer[256] = {0};
            ViUInt32 retCount;
            // Read response
            ViStatus status = viRead(session, (ViBuf)buffer, sizeof(buffer) - 1, &retCount);
            if(status==VI_SUCCESS)
            {
                buffer[retCount] = '\0';
                QString value=QString::fromLatin1(buffer).trimmed();
                m_localVariables.insert(variableName,value);
                QString str="Read success from "+firstParam+": "+value+". Saved to "+variableName;
                emit log(str);

            }
            m_commandCounter++;
            emit startTestingExecuteSignal();

        }
        else if(command=="IQUERYS")
        {
            ViSession session=m_instruments.value(firstParam);
            QString scpiCommand=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            QString variableName=m_parameters.testsInfo.at(m_commandCounter).thirdParam;
            ViChar buffer[256] = {0};
            ViStatus status =instrumentQuery(session,scpiCommand,buffer);
            if(status==VI_SUCCESS)
            {
                QString value=QString::fromLatin1(buffer).trimmed();
                m_localVariables.insert(variableName,value);
                QString str="Query success from "+firstParam+": "+value+". Saved to "+variableName;
                emit log(str);
            }
            m_commandCounter++;
            emit startTestingExecuteSignal();

        }
        else if(command=="ITEST")
        {
            QString value=m_localVariables.value(firstParam);
            float valuef=value.toFloat();
            QString lim1=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            float lim1f=lim1.toFloat();
            QString lim2=m_parameters.testsInfo.at(m_commandCounter).thirdParam;
            float lim2f=lim2.toFloat();

            Test_Output_t output;
            output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
            output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
            QString str;
            if((valuef>lim1f) && (valuef<lim2f))
            {
                output.status="Passed";
                str+="Test passed";
            }
            else
            {
                output.status="Failed";
                str+="Test failed";
            }

            m_pdf.testsTable.append(output);
            str= str+" "+output.testDescription;
            emit log(str);
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else if(command=="IQUERY")
        {
            ViSession session=m_instruments.value(firstParam);
            QString scpiCmd=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            ViChar buffer[256] = {0};
            ViStatus status =instrumentQuery(session,scpiCmd,buffer);
            QString value=QString::fromLatin1(buffer).trimmed();
            QString str="Query success from "+firstParam+": "+value;
            emit log(str);
            m_commandCounter++;
            emit startTestingExecuteSignal();

        }
        else if(command=="IQUERYL")
        {
            emit log("Waiting for instrument");
            m_timer.disconnect();
            connect(&m_timer,&QTimer::timeout,this,&Boot_Control::instrumentQueryLoop,Qt::QueuedConnection);
            m_timer.start(5000);
        }
        else if(command=="IQPLOTBD")
        {
            ViSession session=m_instruments.value(firstParam);
            QString scpiCmd=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            QString plotName=m_parameters.testsInfo.at(m_commandCounter).thirdParam;
            QByteArray data;
            readBlockData(session,scpiCmd,data);
            parseBlockDataFormat1(data,plotName,m_plots);
            QString str="Block data read success from "+firstParam;
            emit log(str);
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
    }


    //emit autoBootFinishedSignal();
}


void Boot_Control::mainFirmware1Dump()
{
    putMCUInBootloaderMode();
    m_currentMainTask="Main_firmware1_dump";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.mainFirm1Parameter.enabled)
    {

        emit mainFirmware1DumpExecuteSignal();
    }
    else
    {
        emit log("Main firmware1 dump skipped");
        emit startMainFirmware2DumpSignal();
    }
}

void Boot_Control::mainFirmware1DumpExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -w ")+m_parameters.mainFirm1Parameter.filePath
                  + " " +m_parameters.mainFirm1Parameter.startAddress + " -v";

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Dumping main firmware1");
    m_currentBackgroundTask="Main_firmware1_dump";
    m_process.startCommand(cmd);
}

void Boot_Control::mainFirmware1DumpExecuteCompleted()
{
    qInfo()<<"Main firmware1 dump execution completed";
    if(m_process.exitCode()==0)
    {
        qInfo()<<"Main firmware1 dumped";
        emit log("Main firmware1 dumped successfully");
        emit startMainFirmware2DumpSignal();
    }
    else
    {
        qInfo()<<"Main firmware1 dump failed";
        emit log("Main firmware1 dump failed");
        emit autoBootFinishedSignal(); //exit for now
    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::mainFirmware2Dump()
{
    m_currentMainTask="Main_firmware2_dump";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }
    if(m_parameters.mainFirm2Parameter.enabled)
    {
        emit mainFirmware2DumpExecuteSignal();
    }
    else
    {
        emit log("Main firmware2 dump skipped");
        // emit autoBootFinishedSignal();
        emit checkDeviceIdSignal();
    }
}

void Boot_Control::mainFirmware2DumpExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -w ")+m_parameters.mainFirm2Parameter.filePath
                  + " " +m_parameters.mainFirm2Parameter.startAddress + " -v";

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Dumping main firmware2");
    m_currentBackgroundTask="Main_firmware2_dump";
    m_process.startCommand(cmd);
}

void Boot_Control::mainFirmware2DumpExecuteCompleted()
{
    qInfo()<<"Main firmware2 dump execution completed";
    if(m_process.exitCode()==0)
    {
        qInfo()<<"Main firmware2 dumped";
        emit log("Main firmware2 dumped successfully");
        // emit autoBootFinishedSignal();
        emit checkDeviceIdSignal();
    }
    else
    {
        qInfo()<<"Main firmware2 dump failed";
        emit log("Main firmware2 dump failed");
        emit autoBootFinishedSignal(); //exit for now
    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::checkDeviceId()
{
    m_currentMainTask="Program_Device_ID";
    if(m_userTerminated)
    {

        emit userTerminationSignal();
        return;
    }

    if(m_parameters.deviceId.programDeviceId)
    {
        emit checkDeviceIdExecuteSignal();
    }
    else
    {
        emit log("Program device ID skipped");
        // emit autoBootFinishedSignal();
        m_pdf.header["Product ID"]="0x11111111";
        emit pdfExportStartSignal();
    }
}

void Boot_Control::checkDeviceIdExecute()
{
    qInfo()<<"Device start address: "<< m_parameters.deviceId.startAddress;
    //qInfo()<<"Temp dir: "<<m_tempDir.path();
    // QDir dir(m_tempDir.path());
    m_tempFilePath=QDir::toNativeSeparators(m_tempDir.path())+ QDir::separator()+"read.bin";
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -r ")+m_parameters.deviceId.startAddress+" 4096 "+
                  m_tempFilePath;

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Checking device ID");
    m_currentBackgroundTask="Check_Device_ID";
    m_process.startCommand(cmd);
}

void Boot_Control::checkDeviceIdExecuteCompleted()
{
    qInfo()<<"Check device ID execution completed";
    if(m_process.exitCode()==0)
    {
        QFile file(m_tempFilePath);
        if(file.exists())
        {

            if(file.open(QIODevice::ReadWrite))
            {
                quint32 deviceId;
                quint8 productName[20];
                file.read((char*)&deviceId,4);
                file.read((char*)productName,20);
                qInfo()<<"Device ID: "<<deviceId;
                QString productNameStr=QString::fromUtf8((char*)productName,20).trimmed();
                qInfo()<<"Device name: "<<productNameStr;
                m_pdf.header["Product Name"]=productNameStr;

                QString deviceIdStr=QString::number(deviceId,16);
                qInfo()<<"Device ID string: "<<deviceIdStr;
                if(("0x"+deviceIdStr)==m_unprogrammedDeviceID)
                {
                    emit log("Device ID not programmed");
                    deviceIdStr=m_parameters.deviceId.deviceId;
                    // deviceIdStr=deviceIdStr.toLower();
                    // qInfo()<<"Lower case: "<<deviceIdStr;
                    // int index=deviceIdStr.indexOf("x");
                    // qInfo()<<"Index of x: "<<index;
                    // deviceIdStr=deviceIdStr.mid(index+1);
                    // qInfo()<<"After removing x: "<<deviceIdStr;
                    deviceId=deviceIdStr.toUInt(0,16);
                    qInfo()<<"Modified device id: "<<deviceId;
                    file.seek(0);
                    file.write((char*)&deviceId,4);
                    // file.close();
                    emit programDeviceIdSignal();
                }
                else
                {
                    // file.close();
                    emit log("Device ID already programmed");
                    m_pdf.header["Product ID"]="0x"+deviceIdStr;
                    // emit autoBootFinishedSignal(); //exit for now
                    emit pdfExportStartSignal(); //export pdf start signal
                }
                //if()
                //file.write((char*)&deviceId,4);
                file.close();

            }


        }
        // if(m_receivedDetails.contains(m_parameters.deviceId.startAddress))
        // {
        //     // m_pdf.header["MCU ID"]=m_receivedDetails.value(m_parameters.deviceId.startAddress);
        //     QString value="0x"+m_receivedDetails.value(m_parameters.deviceId.startAddress);
        //     qInfo()<<"Device ID: "<<value;
        //     if(value==m_unprogrammedDeviceID)
        //     {
        //         emit log("Device ID not programmed");
        //         emit programDeviceIdSignal();
        //     }
        //     else
        //     {
        //         emit log("Device ID already programmed");
        //         emit autoBootFinishedSignal(); //exit for now
        //     }

        // }
        // else
        // {
        //     emit log("Failed to get device ID");
        //     emit autoBootFinishedSignal(); //exit for now
        // }

    }
    else
    {
        emit autoBootFinishedSignal(); //exit for now
    }
    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::programDeviceId()
{
    // emit autoBootFinishedSignal(); //exit for now
    emit programDeviceIdExecuteSignal();
}

void Boot_Control::programDeviceIdExecute()
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+m_serialPort+
                  QString(" br=115200 -w ")+m_tempFilePath+" "+m_parameters.deviceId.startAddress;

    if(m_receivedDetails.size()>0)
    {
        m_receivedDetails.clear();
    }
    emit log("Programming device ID: "+m_parameters.deviceId.deviceId);
    m_currentBackgroundTask="Program_Device_ID";
    m_process.startCommand(cmd);
}

void Boot_Control::programDeviceIdExecuteCompleted()
{
    if(m_process.exitCode()==0)
    {
        emit log("Device ID programmed: "+m_parameters.deviceId.deviceId);
        m_pdf.header["Product ID"]=m_parameters.deviceId.deviceId;
        quint32 id=m_parameters.deviceId.deviceId.toUInt(0,16);
        id++;
        QString idStr="0x"+QString::number(id,16);
        m_parameters.deviceId.deviceId=idStr;
        emit updateDeviceIDSignal(idStr);
        // emit autoBootFinishedSignal(); //exit for now

    }
    else
    {
        m_pdf.header["Product ID"]="0x11111111";
        // emit autoBootFinishedSignal(); //exit for now
    }
    emit pdfExportStartSignal(); //export pdf start signal

    m_updateReadProgressEnabled=false;
    m_updateReadProgressEnabled=false;
}

void Boot_Control::autoBootFinished()
{
    if(m_stlinkBridgeConnected)
    {
        exitMCUFromBootloaderMode();
        QString str= QString("Waiting for PCB ")+QString::number(m_PCBCount)+QString(" to disconnect");
        emit log(str);
        disconnect(&m_timer,&QTimer::timeout,0,0);
        connect(&m_timer,&QTimer::timeout,this,&Boot_Control::detectPCBDisconnection,Qt::QueuedConnection);
        m_timer.start(m_GPIODetectDelay);
    }
    else
    {
        QString str= QString("Restart after placing MCU in boot mode");
        emit log(str);
    }

}

void Boot_Control::receivedCommand(QString command)
{
    m_timer.start(m_commandTimeout); //restart command timeout timer
    // emit log(QString("Command received: ")+command);
    if(m_expectedCmdResponse.second.first=="CMD_ACK")
    {
        qInfo()<<"In ack";
        if(command==m_expectedCmdResponse.second.first)
        {
            if(m_expectedCmdResponse.first=="TEST_BLE")
            {
                m_expectedCmdResponse.second.first=m_parameters.testsInfo.at(m_commandCounter).secondParam;
            }
            else
            {
                m_expectedCmdResponse.second.first=m_parameters.testsInfo.at(m_commandCounter).secondParam;
                m_expectedCmdResponse.second.second=m_parameters.testsInfo.at(m_commandCounter).thirdParam;
                // qInfo()<<"Received ack";
                // emit log(QString("Command received: ")+command);
            }
            QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
            emit log(str+" acknowledged");
        }
        else if(command==m_expectedCmdResponse.second.second)
        {
            //test skipped
            qInfo()<<"Received nack-> test skipped";
            QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
            emit log(str+" rejected");
            emit log("Skipping the "+str+"\n");
            Test_Output_t output;
            output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
            output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
            output.status="Skipped";
            m_pdf.testsTable.append(output);
            m_expectedCmdResponse.first="None";
            m_expectedCmdResponse.second.first="None";
            m_expectedCmdResponse.second.second="None";
            m_commandCounter++;
            emit startTestingExecuteSignal();
        }
        else
        {
            //nothing for now
        }
    }
    else
    {
        if(m_expectedCmdResponse.first=="TESTBLE") //if command is BLE testing
        {
            qInfo()<<"In test BLE";
            if(command==m_expectedCmdResponse.second.first)
            {
                //BLE testing
                //start scan for BLE device
                m_bleDeviceScan.first=m_parameters.testsInfo.at(m_commandCounter).thirdParam; //ble device to scan for
                m_bleDeviceScan.second=false;
                m_bleScanner.disconnect();
                connect(&m_bleScanner,&BLE::bleScanFinishedSignal,this,&Boot_Control::bleScanFinished,Qt::QueuedConnection);
                connect(&m_bleScanner,&BLE::addDiscoveredBLEdevicesSignal,this,&Boot_Control::discoveredBLEdevice,Qt::QueuedConnection);
                m_bleScanner.startBLEdiscovery();
                m_timer.disconnect();
                connect(&m_timer,&QTimer::timeout,this,&Boot_Control::bleScanTimeout,Qt::QueuedConnection);
                m_timer.start(m_bleScanTimeout);
                //test passed if ble device found deviceName==m_parameters.testsInfo.at(m_commandCounter).nResponse
                // m_commandCounter++;
                // m_expectedCmdResponse.first="None";
                // m_expectedCmdResponse.second.first="None";
                // m_expectedCmdResponse.second.second="None";
                // emit startTestingExecuteSignal();
            }
        }
        else
        {
            if(command==m_expectedCmdResponse.second.first)
            {
                //test passed
                qInfo()<<"Test passed";
                QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
                emit log(str+" passed"+"\n");

                Test_Output_t output;
                output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
                output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
                output.status="Passed";
                m_pdf.testsTable.append(output);

                m_commandCounter++;
                m_expectedCmdResponse.first="None";
                m_expectedCmdResponse.second.first="None";
                m_expectedCmdResponse.second.second="None";
                emit startTestingExecuteSignal();

            }
            else if(command==m_expectedCmdResponse.second.second)
            {
                //test failed
                qInfo()<<"Test failed";
                QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
                emit log(str+" failed"+"\n");

                Test_Output_t output;
                output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
                output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
                output.status="Failed";
                m_pdf.testsTable.append(output);

                m_commandCounter++;
                m_expectedCmdResponse.first="None";
                m_expectedCmdResponse.second.first="None";
                m_expectedCmdResponse.second.second="None";
                emit startTestingExecuteSignal();

            }
        }

    }

    // if(command=="CNTRLR_READY")
    // {


    //     m_cmdlist.append(command);
    // }
    // else if(command=="PCB_CONNECTED")
    // {
    //     if(m_cmdlist.at(0)=="CNTRLR_READY")
    //     {
    //         m_cmdlist.append(command);
    //         sendCommand("START_BOOT\n");
    //     }
    // }
    // else if(command=="BOOTLOADER_STARTED")
    // {
    //     //MCU in bootloader mode
    //     emit log("Starting code dump using UART bootloader");

    //     if(m_samePort)
    //     {
    //         emit closePort();
    //     }
    //     else
    //     {
    //         startProgrammer(m_firmwarePort);
    //     }

    // }

    // else if(command=="BOOTLOADER_STOPPED")
    // {

    // }
    // else
    // {
    //     //nothing for now
    // }
}

void Boot_Control::portClosed()
{
    // if(m_)
    startProgrammer(m_serialPort);
}

void Boot_Control::portOpened(QString portName)
{
    sendCommand("STOP_BOOT\n");
}

void Boot_Control::updateBinDir(QString binDir)
{
    m_binDir=binDir;
}


void Boot_Control::sendCommand(QString cmd)
{
    QByteArray cmdB;
    // cmdB.append(m_uartAddr);
    cmdB.append(cmd.toUtf8());
    emit sendCommandSignal(cmdB);
    // emit log(QString("Command sent: ")+cmd.trimmed());
}

void Boot_Control::readyReadProcessError()
{
    QByteArray error=m_process.readAllStandardError();
    emit log(QString::fromUtf8(error));
}

void Boot_Control::readyReadProcessOutput()
{
    QByteArray output=m_process.readAllStandardOutput();
    // qInfo()<<QString(output);
    // qInfo()<<"\nFinish size: "<<output.size()<<"\n";
    // emit log(QString::fromUtf8(output));
    while(output.size()>0)
    {
        QByteArray data;
        int index=output.indexOf('\n');
        if(index>-1)
        {
            data = output.left(index+1);

            // Remove the read data from the buffer
            output.remove(0, index+1);
        }
        else
        {
            data=output.left(output.size());
            output.remove(0, output.size());
        }
        // emit log(QString::fromUtf8(data));
        // qInfo()<<"Line read: "<<QString(data);
        processCommands(data);

    }



}

void Boot_Control::errorOccurred(QProcess::ProcessError error)
{
    qInfo()<<"Error occurred: "<<error;
}

void Boot_Control::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // qInfo()<<"Exit status: "<<exitStatus<<" Exit code: "<<exitCode;
    // emit log(QString("Process exited code: ")+QString::number(exitCode));
    // if(exitCode==0)
    // {
    //     if(m_samePort)
    //     {
    //         emit openPort();
    //     }
    //     else
    //     {
    //         sendCommand("STOP_BOOT");
    //     }

    // }
    // qInfo()<<"Found commands result: "<<m_commandsRes;
    // if(m_samePort)
    // {
    //     emit openPort();
    // }
    // else
    // {
    //     sendCommand("STOP_BOOT");
    // }
    // if(m_bootModeCheck)
    // {
    //     m_bootModeCheck=false;
    //     emit checkMCUBootloaderModeCompletedSignal();

    // }
    if(m_currentBackgroundTask=="Boot_Mode_Check")
    {
        m_currentBackgroundTask="None";
        // if(m_commandToCheck.contains(toCheck))
        // {
        //     qInfo()<<"Found"<<splitStr;
        //     m_deviceDetails.append(text);
        //     // emit log(text);
        // }
        // else if(toCheck==m_uinqueIdAddress)
        // {
        //     QString str="Unique Chip ID: ";
        //     // emit log(str+splitStr.at(1).trimmed()+QString("\n"));
        //     m_deviceDetails.append(str+splitStr.at(1).trimmed());
        //     qInfo()<<"Found unique ID: "<<splitStr.at(1);
        // }
        emit checkMCUBootloaderModeCompletedSignal();
    }
    else if(m_currentBackgroundTask=="FUS_Start")
    {
        m_currentBackgroundTask="None";
        emit startFUSCommandCompletedSignal();
    }
    else if(m_currentBackgroundTask=="FUS_Update")
    {
        m_currentBackgroundTask="None";
        emit FUSExecutionCompletedSignal();
    }
    else if(m_currentBackgroundTask=="WStack_dump")
    {
        m_currentBackgroundTask="None";
        emit wStackDumpCompletedSignal();
    }
    else if(m_currentBackgroundTask=="WStack_Start")
    {
        m_currentBackgroundTask="None";
        emit runWirelessStackCompletedSignal();
    }
    else if(m_currentBackgroundTask=="Test_firmware_dump")
    {
        m_currentBackgroundTask="None";
        emit testFirmwareDumpExecuteCompletedSignal();
    }
    else if(m_currentBackgroundTask=="Start_User_App")
    {
        m_currentBackgroundTask="None";
        emit startUserAppFinishedSignal();
    }
    else if(m_currentBackgroundTask=="Main_firmware1_dump")
    {
        m_currentBackgroundTask="None";
        emit mainFirmware1DumpExecuteCompletedSignal();
    }
    else if(m_currentBackgroundTask=="Main_firmware2_dump")
    {
        m_currentBackgroundTask="None";
        emit mainFirmware2DumpExecuteCompletedSignal();
    }
    else if(m_currentBackgroundTask=="Check_Device_ID")
    {
        m_currentBackgroundTask="None";
        emit checkDeviceIdExecuteCompletedSignal();
    }
    else if(m_currentBackgroundTask=="Program_Device_ID")
    {
        m_currentBackgroundTask="None";
        emit programDeviceIdExecuteCompletedSignal();
    }
    else
    {
        //nothing for now
    }

}

void Boot_Control::startProgrammer(QString port)
{
    QString cmd=QString("STM32_Programmer_CLI -c port=")+port+
                  QString(" br=115200 -w ")+m_binDir+QString(" 0x08000000 -v");
    QString cmd2=QString("STM32_Programmer_CLI -c port=")+port+" -fusgetstate";
    emit log(cmd2);
    // m_process.startCommand(cmd);
    if(m_commandsRes.size()>0)
    {
        m_commandsRes.clear();
    }
    m_process.startCommand(cmd2);
}

void Boot_Control::processCommands(QString text)
{
    text=text.trimmed();
    QStringList splitStr=text.split(':');
    // qInfo()<<splitStr;
    QString toCheck=splitStr.at(0);
    toCheck=toCheck.trimmed();
    if(splitStr.count()==2)
    {

        // foreach (QString str, m_commandToCheck)
        // {

        //     if(toCheck==str)
        //     {
        //         QString result=splitStr.at(1);
        //         result=result.trimmed();
        //         QStringList cmds;
        //         cmds.append(toCheck);
        //         cmds.append(result);
        //         m_commandsRes.append(cmds);
        //     }
        // }
        if(splitStr.at(0)=="Download in Progress")
        {
            m_updateWriteProgressEnabled=true;
            if(m_currentBackgroundTask=="FUS_Update")
            {
                emit log("FUS update progress: 0%");
            }
            else if(m_currentBackgroundTask=="WStack_dump")
            {
                emit log("Wireless stack dump progres: 0%");
            }
            else if(m_currentBackgroundTask=="Test_firmware_dump")
            {
                emit log("Test firmware dump progress: 0%");
            }
            else if(m_currentBackgroundTask=="Main_firmware1_dump")
            {
                emit log("Main firmware1 dump progress: 0%");
            }
            else if(m_currentBackgroundTask=="Main_firmware2_dump")
            {
                emit log("Main firmware2 dump progress: 0%");
            }
        }
        else if(splitStr.at(0)=="Read progress")
        {
            m_updateWriteProgressEnabled=false;
            m_updateReadProgressEnabled=true;
            if(m_currentBackgroundTask=="FUS_Update")
            {
                emit log("FUS update verify progress: 0%");
            }
            else if(m_currentBackgroundTask=="WStack_dump")
            {
                emit log("Stack dump verify progress: 0%");
            }
            else if(m_currentBackgroundTask=="Test_firmware_dump")
            {
                emit log("Test firmware dump verify progress: 0%");
            }
            else if(m_currentBackgroundTask=="Main_firmware1_dump")
            {
                emit log("Main firmware1 dump verify progress: 0%");
            }
            else if(m_currentBackgroundTask=="Main_firmware2_dump")
            {
                emit log("Main firmware2 dump verify progress: 0%");
            }
        }
        m_receivedDetails[splitStr.at(0).trimmed()]=splitStr.at(1).trimmed();

    }
    else if(splitStr.count()==1)
    {
        if(m_updateWriteProgressEnabled)
        {
            QString str=splitStr.at(0);
            if(str.contains('%'))
            {
                QString substr=str.right(4).trimmed();
                // qInfo()<<"Write Progress: "<<substr;
                if(m_currentBackgroundTask=="FUS_Update")
                {
                    emit updateLog("FUS update progress: "+substr);
                }
                else if(m_currentBackgroundTask=="WStack_dump")
                {
                    emit updateLog("Wireless stack dump progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Test_firmware_dump")
                {
                    emit updateLog("Test firmware dump progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Main_firmware1_dump")
                {
                    emit updateLog("Main firmware1 dump progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Main_firmware2_dump")
                {
                    emit updateLog("Main firmware2 dump progress: "+substr);
                }
                else
                {

                }
            }

        }
        else if(m_updateReadProgressEnabled)
        {
            QString str=splitStr.at(0);
            if(str.contains('%'))
            {
                QString substr=str.right(4).trimmed();
                // qInfo()<<"Read Progress: "<<substr;
                if(m_currentBackgroundTask=="FUS_Update")
                {
                    emit updateLog("FUS update verify progress: "+substr);
                }
                else if(m_currentBackgroundTask=="WStack_dump")
                {
                    emit updateLog("Stack dump verify progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Test_firmware_dump")
                {
                    emit updateLog("Test firmware dump verify progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Main_firmware1_dump")
                {
                    emit updateLog("Main firmware1 dump verify progress: "+substr);
                }
                else if(m_currentBackgroundTask=="Main_firmware2_dump")
                {
                    emit updateLog("Main firmware2 dump verify progress: "+substr);
                }
                else
                {

                }
            }

        }
        else
        {
            //nothing for now
        }
    }
}

void Boot_Control::userTermination()
{
    if(m_userTerminated)
    {
        m_userTerminated=false;
        if(m_timer.isActive())
        {
            m_timer.stop();
        }
        emit log("Auto boot stopped by the user");
    }

}

QString Boot_Control::getVersionFromHex(QString version)
{
    //version=0x01020000 -> 1.2.0
    int major=version.mid(2,2).toUInt();
    int minor=version.mid(4,2).toUInt();
    int other=version.mid(6,2).toUInt();

    return QString::number(major)+"."+QString::number(minor)+"."+QString::number(other);

}

void Boot_Control::bleScanFinished()
{
    m_timer.stop();
    // qInfo()<<"BLE scan finished";
    if(m_bleDeviceScan.second==true)
    {
        //test passed
        qInfo()<<"Test passed";
        QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
        emit log(str+" passed"+"\n");

        Test_Output_t output;
        output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
        output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
        output.status="Passed";
        m_pdf.testsTable.append(output);

        m_commandCounter++;
        m_expectedCmdResponse.first="None";
        m_expectedCmdResponse.second.first="None";
        m_expectedCmdResponse.second.second="None";
        // emit startTestingExecuteSignal();
    }
    else
    {
        //test failed
        qInfo()<<"Test failed";
        QString str=m_parameters.testsInfo.at(m_commandCounter).firstParam;
        emit log(str+" failed"+"\n");

        Test_Output_t output;
        output.testDescription=m_parameters.testsInfo.at(m_commandCounter).testDescription;
        output.testReqirement=m_parameters.testsInfo.at(m_commandCounter).testReqirement;
        output.status="Failed";
        m_pdf.testsTable.append(output);

        m_commandCounter++;
        m_expectedCmdResponse.first="None";
        m_expectedCmdResponse.second.first="None";
        m_expectedCmdResponse.second.second="None";
        // emit startTestingExecuteSignal();
    }
    emit startTestingExecuteSignal();
}

void Boot_Control::discoveredBLEdevice(const QBluetoothDeviceInfo &deviceInfo)
{
    QString logStr="Name: "+deviceInfo.name()+" Address: "+deviceInfo.address().toString();
    qInfo()<<logStr;
    // emit log(logStr);
    if(deviceInfo.name()==m_bleDeviceScan.first)
    {
        m_bleDeviceScan.second=true;
    }
}
