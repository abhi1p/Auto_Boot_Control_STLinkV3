#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_toolbar=new QToolBar(this);
    m_MCUSelectComboBox=new QComboBox(this);
    m_MCUSelectComboBox->addItems(m_MCUList);
    // m_portWidget= new Port(this);
    m_MCUSelectLabel=new QLabel(this);
    m_MCUSelectLabel->setText("MCU:  ");

    m_toolbar->addWidget(m_MCUSelectLabel);
    m_toolbar->addWidget(m_MCUSelectComboBox);
    addToolBar(m_toolbar);
    connect(m_MCUSelectComboBox,&QComboBox::currentTextChanged,this,&MainWindow::mcuSelectComboboxSelectionChanged,Qt::QueuedConnection);
    connect(this,&MainWindow::enableDisableMCUSelectionSignal,this,&MainWindow::enableDisableMCUSelection,Qt::QueuedConnection);
    connect(ui->scanPushButton,&QPushButton::clicked,this,&MainWindow::scanAllSerialPorts,Qt::QueuedConnection);
    // connect(ui->startAutoBtn,&QPushButton::clicked,this,&MainWindow::openSerialPort,Qt::QueuedConnection);

    connect(ui->fusLocationBrowseBtn,&QPushButton::clicked,this,&MainWindow::browseFUSFilePath,Qt::QueuedConnection);
    connect(ui->stackFileBrowseBtn,&QPushButton::clicked,this,&MainWindow::browseStackFilePath,Qt::QueuedConnection);
    connect(ui->testFirmFileBrowseBtn,&QPushButton::clicked,this,&MainWindow::browseTestFirmwareFilePath,Qt::QueuedConnection);
    connect(ui->cmdFileBrowseBtn,&QPushButton::clicked,this,&MainWindow::browseCommandsFilePath,Qt::QueuedConnection);
    connect(ui->mainFirmware1BrowseBtn,&QPushButton::clicked,this,&MainWindow::browseMainFirmware1FilePath,Qt::QueuedConnection);
    connect(ui->mainFirmware2BrowseBtn,&QPushButton::clicked,this,&MainWindow::browseMainFirmware2FilePath,Qt::QueuedConnection);
    connect(ui->resultsFileBrowseBtn,&QPushButton::clicked,this,&MainWindow::browseResultsFilePath,Qt::QueuedConnection);

    connect(ui->dummyRunCheckBox,&QCheckBox::stateChanged,this,&MainWindow::dummyRunStateChanged,Qt::QueuedConnection);
    connect(ui->fusUpdateCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableFUSUpdate,Qt::QueuedConnection);
    connect(ui->stackDumpCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableStackDump,Qt::QueuedConnection);
    connect(ui->testCodeDumpCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableTestCodeDump,Qt::QueuedConnection);
    connect(ui->programIDCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableProgramID,Qt::QueuedConnection);
    connect(ui->sendCommandsCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableSendCommands,Qt::QueuedConnection);
    connect(ui->mainCodeDump1CheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableMainCode1Dump,Qt::QueuedConnection);
    connect(ui->mainCodeDump2CheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableMainCode2Dump,Qt::QueuedConnection);
    connect(ui->saveResultsCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableSaveResults,Qt::QueuedConnection);
    connect(ui->saveLogCheckBox,&QCheckBox::stateChanged,this,&MainWindow::enableSaveLog,Qt::QueuedConnection);

    // connect(ui->commandsLineEdit,&QLineEdit::textChanged,this,&MainWindow::parseCommandsBtnClicked,Qt::QueuedConnection);
    connect(ui->parseCommandsBtn,&QPushButton::clicked,this,&MainWindow::parseCommandsBtnClicked,Qt::QueuedConnection);
    connect(ui->startAutoBtn,&QPushButton::clicked,this,&MainWindow::startAutoBootClicked,Qt::QueuedConnection);

    m_port_model=new SerialPortModel(this);
    ui->serialPortComboBox->setModel(m_port_model);
    // ui->firmwarePortComboBox->setModel(m_port_model);

    // m_fileDialog=new QFileDialog(this);
    // m_redirectPath=QDir::homePath()+ QDir::separator()+"STM32Cube"+QDir::separator()+"Repository";


    connect(&m_testTimer,&QTimer::timeout,this,&MainWindow::testTimerTimeout,Qt::QueuedConnection);
    // log("Test");
    // log("Test Progress: 0%");
    // m_testTimer.start(250);;
    // scanAllSerialPorts();

    loadAllSettings();
}

MainWindow::~MainWindow()
{
    // delete m_fileDialog;
    delete m_MCUSelectLabel;
    delete m_MCUSelectComboBox;
    delete m_toolbar;
    // delete m_portWidget;
    delete m_port_model;
    delete ui;
}

void MainWindow::connectUSBSignalAndSlots(USB_Serial *usb_serial)
{
    connect(usb_serial,&USB_Serial::foundSerialPort,this,&MainWindow::foundSerialPort,Qt::QueuedConnection);
    connect(usb_serial,&USB_Serial::serialPortOpenedSignal,this,&MainWindow::serialPortOpened,Qt::QueuedConnection);
    connect(usb_serial,&USB_Serial::log,this,&MainWindow::log,Qt::QueuedConnection);
    connect(this,&MainWindow::scanAllSerialPortsSignal,usb_serial,&USB_Serial::getAllSerialPort,Qt::QueuedConnection);
    connect(this,&MainWindow::openSerialPortSignal,usb_serial,&USB_Serial::openSerialPort,Qt::QueuedConnection);
}

void MainWindow::connectBootSignalAndSlots(Boot_Control *boot)
{
    connect(boot,&Boot_Control::log,this,&MainWindow::log,Qt::QueuedConnection);
    connect(boot,&Boot_Control::updateLog,this,&MainWindow::updateLog,Qt::QueuedConnection);
    connect(boot,&Boot_Control::updateDeviceIDSignal,this,&MainWindow::updateDeviceID,Qt::QueuedConnection);
    connect(this,&MainWindow::startAutoBoot,boot,&Boot_Control::startAutoBoot,Qt::QueuedConnection);
    connect(this,&MainWindow::updateBinDir,boot,&Boot_Control::updateBinDir,Qt::QueuedConnection);
    scanAllSerialPorts();
}

void MainWindow::foundSerialPort(QSerialPortInfo port)
{
    ui->serialPortComboBox->setEnabled(true);
    qInfo()<<"Port found: "<<port.portName();
    m_port_model->addSerialPort(port);
    QString str=QString("Found port: ")+port.portName();
    log(str);

}

void MainWindow::serialPortOpened(bool opened)
{
#if 0
    if(opened)
    {
        qInfo()<<"Serial port opened";
        log("Serial port opened");
        // log("\nWaiting for button pressed");
        m_portOpened=true;
        ui->startAutoBtn->setText("Stop");

        ///Port model index for command port
        int currIndex=ui->serialPortComboBox->currentIndex();
        QModelIndex portModelIndex=m_port_model->index(currIndex,0);
        // qInfo()<<"Model index: "<<portModelIndex;
        QSerialPortInfo commandPortInfo=m_port_model->getSerialPortInfo(portModelIndex);

        ///Port model index for firmware port
        // currIndex=ui->firmwarePortComboBox->currentIndex();
        portModelIndex=m_port_model->index(currIndex,0);
        // qInfo()<<"Model index: "<<portModelIndex;
        QSerialPortInfo firmwarePortInfo=m_port_model->getSerialPortInfo(portModelIndex);


        emit startAutoBoot(true,commandPortInfo.portName(),firmwarePortInfo.portName());

        ui->testCodeDumpCheckBox->setEnabled(false);
        ui->scanPushButton->setEnabled(false);
        ui->autoCheckBox->setEnabled(false);
        ui->serialPortComboBox->setEnabled(false);
        // ui->firmwarePortComboBox->setEnabled(false);
    }
    else
    {
        qInfo()<<"Serial port closed";
        log("Serial port closed");
        // if(!)
        m_portOpened=false;
        ui->startAutoBtn->setText("Start");
        ui->testCodeDumpCheckBox->setEnabled(true);
        ui->scanPushButton->setEnabled(true);
        ui->autoCheckBox->setEnabled(true);
        ui->serialPortComboBox->setEnabled(true);
        // ui->firmwarePortComboBox->setEnabled(true);
    }
#endif

}

void MainWindow::receivedCommand(QString command)
{
    QString text=QString("Received command: ")+command;
    qInfo()<<text;
    log(text);
}

void MainWindow::log(QString text)
{
    ui->logDisplayPlainTextEdit->appendPlainText(text);
    // ui->logDisplayPlainTextEdit->appendPlainText("\n");
}

void MainWindow::updateLog(QString text)
{
    // qInfo()<<"Overrite mode enabled: "<<ui->logDisplayPlainTextEdit->overwriteMode();
    // ui->logDisplayPlainTextEdit->setOverwriteMode(true);
    QTextCursor cursor=ui->logDisplayPlainTextEdit->textCursor();
    // cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1);
    cursor.insertText(text);
    ui->logDisplayPlainTextEdit->setTextCursor(cursor);
    // cursor.
    // ui->logDisplayPlainTextEdit->s
    // ui->logDisplayPlainTextEdit->insertPlainText(text);
    // cursor.
    // cursor.insertText(text);
    // ui->logDisplayPlainTextEdit->setOverwriteMode(false);
}

void MainWindow::updateDeviceID(QString deviceID)
{
    ui->deviceIdLineEdit->setText(deviceID);
}

void MainWindow::openSerialPort()
{

#if 1
    if(m_testFirmwareFileSelected)
    {

        if(!m_portOpened)
        {
            log("Start button clicked");

            // int currIndex=ui->serialPortComboBox->currentIndex();
            // QModelIndex portModelIndex=m_port_model->index(currIndex,0);
            // qInfo()<<"Model index: "<<portModelIndex;
            // QSerialPortInfo portInfo=m_port_model->getSerialPortInfo(portModelIndex);
            // QString text=QString("Selected port: ")+portInfo.portName();
            // qInfo()<<text;
            // log(text);

            // emit openSerialPortSignal(true,portInfo,m_baudRate,m_parity,m_dataBits,m_stopBits);

        }
        else
        {
            emit openSerialPortSignal(false);

        }
    }
    else
    {
        log("Empty file path or invalid file path");
    }

#endif

}

void MainWindow::scanAllSerialPorts()
{
    m_port_model->clear();
    emit scanAllSerialPortsSignal();
}

void MainWindow::enableTestCodeDump(int state)
{
    //qInfo()<<"Code dump: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->testFirmLineEdit->text().isEmpty())
        {
            if(!ui->testFirmwareStartAddrLineEdit->text().isEmpty())
            {
                bool validFile=QFile::exists(ui->testFirmLineEdit->text());
                if(validFile)
                {
                    ui->testFirmFileBrowseBtn->setEnabled(false);
                    ui->testFirmLineEdit->setEnabled(false);
                    ui->testFirmwareStartAddrLineEdit->setEnabled(false);
                    m_testFirmwareFileSelected=true;
                    emit updateBinDir(ui->testFirmLineEdit->text());
                }
                else
                {
                    m_testFirmwareFileSelected=false;
                    ui->testFirmFileBrowseBtn->setEnabled(true);
                    ui->testFirmLineEdit->setEnabled(true);
                    ui->testFirmwareStartAddrLineEdit->setEnabled(true);
                    ui->testCodeDumpCheckBox->blockSignals(true);
                    ui->testCodeDumpCheckBox->setCheckState(Qt::Unchecked);
                    ui->testCodeDumpCheckBox->blockSignals(false);
                    log("Invalid file path");

                }
            }
            else
            {
                m_testFirmwareFileSelected=false;
                ui->testFirmFileBrowseBtn->setEnabled(true);
                ui->testFirmLineEdit->setEnabled(true);
                ui->testFirmwareStartAddrLineEdit->setEnabled(true);
                ui->testCodeDumpCheckBox->blockSignals(true);
                ui->testCodeDumpCheckBox->setCheckState(Qt::Unchecked);
                ui->testCodeDumpCheckBox->blockSignals(false);
                log("Empty start address");
            }


        }
        else
        {
            ui->testFirmFileBrowseBtn->setEnabled(true);
            ui->testFirmLineEdit->setEnabled(true);
            ui->testFirmwareStartAddrLineEdit->setEnabled(true);
            ui->testCodeDumpCheckBox->blockSignals(true);
            ui->testCodeDumpCheckBox->setCheckState(Qt::Unchecked);
            ui->testCodeDumpCheckBox->blockSignals(false);
            m_testFirmwareFileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->testFirmFileBrowseBtn->setEnabled(true);
        ui->testFirmwareStartAddrLineEdit->setEnabled(true);
        ui->testFirmLineEdit->setEnabled(true);
        m_testFirmwareFileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}

void MainWindow::enableSaveResults(int state)
{
    //qInfo()<<"Save results: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->resultsFileLocLineEdit->text().isEmpty())
        {
            // bool fileExist=QFile::exists(ui->resultsFileLocLineEdit->text());
            // if(fileExist)
            // {
            //     log("Results will be overwritten");
            // }
            // else
            // {
            //     ui->resultsFileBrowseBtn->setEnabled(true);
            //     ui->resultsFileLocLineEdit->setEnabled(true);
            //     ui->saveResultsCheckBox->blockSignals(true);
            //     ui->saveResultsCheckBox->setCheckState(Qt::Unchecked);
            //     ui->saveResultsCheckBox->blockSignals(false);
            //     log("Invalid file path");

            // }
            ui->resultsFileBrowseBtn->setEnabled(false);
            ui->resultsFileLocLineEdit->setEnabled(false);


        }
        else
        {
            ui->resultsFileBrowseBtn->setEnabled(true);
            ui->resultsFileLocLineEdit->setEnabled(true);
            ui->saveResultsCheckBox->blockSignals(true);
            ui->saveResultsCheckBox->setCheckState(Qt::Unchecked);
            ui->saveResultsCheckBox->blockSignals(false);
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->resultsFileBrowseBtn->setEnabled(true);
        ui->resultsFileLocLineEdit->setEnabled(true);
    }
}

void MainWindow::enableSaveLog(int state)
{
    if(state==Qt::Checked)
    {
        m_saveLog=true;
    }
    else if(state==Qt::Unchecked)
    {
        m_saveLog=false;
    }

}
void MainWindow::enableFUSUpdate(int state)
{
    //qInfo()<<"FUS update: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->fusLocationLineEdit->text().isEmpty())
        {
            if(!ui->fusStartAddrLineEdit->text().isEmpty())
            {
                bool validFile=QFile::exists(ui->fusLocationLineEdit->text());
                if(validFile)
                {
                    ui->fusLocationBrowseBtn->setEnabled(false);
                    ui->fusLocationLineEdit->setEnabled(false);
                    ui->fusStartAddrLineEdit->setEnabled(false);
                    m_fusFileSelected=true;


                }
                else
                {
                    m_fusFileSelected=false;
                    ui->fusLocationBrowseBtn->setEnabled(true);
                    ui->fusLocationLineEdit->setEnabled(true);
                    ui->fusStartAddrLineEdit->setEnabled(true);
                    ui->fusUpdateCheckBox->blockSignals(true);
                    ui->fusUpdateCheckBox->setCheckState(Qt::Unchecked);
                    ui->fusUpdateCheckBox->blockSignals(false);
                    log("Invalid file path");

                }
            }
            else
            {
                m_fusFileSelected=false;
                ui->fusLocationBrowseBtn->setEnabled(true);
                ui->fusLocationLineEdit->setEnabled(true);
                ui->fusStartAddrLineEdit->setEnabled(true);
                ui->fusUpdateCheckBox->blockSignals(true);
                ui->fusUpdateCheckBox->setCheckState(Qt::Unchecked);
                ui->fusUpdateCheckBox->blockSignals(false);
                log("Empty start address");
            }



        }
        else
        {
            ui->fusLocationBrowseBtn->setEnabled(true);
            ui->fusLocationLineEdit->setEnabled(true);
            ui->fusStartAddrLineEdit->setEnabled(true);
            ui->fusUpdateCheckBox->blockSignals(true);
            ui->fusUpdateCheckBox->setCheckState(Qt::Unchecked);
            ui->fusUpdateCheckBox->blockSignals(false);
            m_fusFileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->fusLocationBrowseBtn->setEnabled(true);
        ui->fusLocationLineEdit->setEnabled(true);
        ui->fusStartAddrLineEdit->setEnabled(true);
        m_fusFileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}
void MainWindow::enableStackDump(int state)
{
    //qInfo()<<"Stack dump: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->wStackLineEdit->text().isEmpty())
        {
            if(!ui->wStackStartAddrLineEdit->text().isEmpty())
            {
                bool validFile=QFile::exists(ui->wStackLineEdit->text());
                if(validFile)
                {
                    ui->stackFileBrowseBtn->setEnabled(false);
                    ui->wStackLineEdit->setEnabled(false);
                    ui->wStackStartAddrLineEdit->setEnabled(false);
                    m_stackFileSelected=true;

                }
                else
                {
                    m_stackFileSelected=false;
                    ui->stackFileBrowseBtn->setEnabled(true);
                    ui->wStackLineEdit->setEnabled(true);
                    ui->wStackStartAddrLineEdit->setEnabled(true);
                    ui->stackDumpCheckBox->blockSignals(true);
                    ui->stackDumpCheckBox->setCheckState(Qt::Unchecked);
                    ui->stackDumpCheckBox->blockSignals(false);
                    log("Invalid file path");

                }
            }
            else
            {
                m_stackFileSelected=false;
                ui->stackFileBrowseBtn->setEnabled(true);
                ui->wStackLineEdit->setEnabled(true);
                ui->wStackStartAddrLineEdit->setEnabled(true);
                ui->stackDumpCheckBox->blockSignals(true);
                ui->stackDumpCheckBox->setCheckState(Qt::Unchecked);
                ui->stackDumpCheckBox->blockSignals(false);
                log("Empty start address");
            }

        }
        else
        {
            ui->stackFileBrowseBtn->setEnabled(true);
            ui->wStackLineEdit->setEnabled(true);
            ui->wStackStartAddrLineEdit->setEnabled(true);
            ui->stackDumpCheckBox->blockSignals(true);
            ui->stackDumpCheckBox->setCheckState(Qt::Unchecked);
            ui->stackDumpCheckBox->blockSignals(false);
            m_stackFileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->stackFileBrowseBtn->setEnabled(true);
        ui->wStackLineEdit->setEnabled(true);
        ui->wStackStartAddrLineEdit->setEnabled(true);
        m_stackFileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}

void MainWindow::enableProgramID(int state)
{
    if(state==Qt::Checked)
    {
        if((!ui->devIdAddress->text().isEmpty()) && (!ui->deviceIdLineEdit->text().isEmpty()))
        {
            QString address=ui->devIdAddress->text();
            quint32 value=address.toInt(0,16);
            // qInfo()<<"Address: "<<QString::number(value,16);
            if(m_MCUSelectComboBox->currentText()=="STM32WB1x")
            {
                if(value%2==0)
                {
                    m_programID=true;
                    ui->devIdAddress->setEnabled(false);
                    ui->deviceIdLineEdit->setEnabled(false);
                }
                else
                {
                    log("Not valid address. Address must be start address of page/sector");
                    m_programID=false;
                    ui->devIdAddress->setEnabled(true);
                    ui->deviceIdLineEdit->setEnabled(true);
                    ui->programIDCheckBox->blockSignals(true);
                    ui->programIDCheckBox->setCheckState(Qt::Unchecked);
                    ui->programIDCheckBox->blockSignals(false);
                }
            }
            else if(m_MCUSelectComboBox->currentText()=="None")
            {
                m_programID=true;
                ui->devIdAddress->setEnabled(false);
                ui->deviceIdLineEdit->setEnabled(false);
            }
            else
            {
                if(value%4==0)
                {
                    m_programID=true;
                    ui->devIdAddress->setEnabled(false);
                    ui->deviceIdLineEdit->setEnabled(false);
                }
                else
                {
                    log("Not valid address. Address must be start address of page/sector");
                    m_programID=false;
                    ui->devIdAddress->setEnabled(true);
                    ui->deviceIdLineEdit->setEnabled(true);
                    ui->programIDCheckBox->blockSignals(true);
                    ui->programIDCheckBox->setCheckState(Qt::Unchecked);
                    ui->programIDCheckBox->blockSignals(false);
                }
            }
        }
        else
        {
            log("DeviceID and/or Address empty");
            m_programID=false;
            ui->devIdAddress->setEnabled(true);
            ui->deviceIdLineEdit->setEnabled(true);
            ui->programIDCheckBox->blockSignals(true);
            ui->programIDCheckBox->setCheckState(Qt::Unchecked);
            ui->programIDCheckBox->blockSignals(false);
        }
    }
    else if(state==Qt::Unchecked)
    {

        m_programID=false;
        ui->devIdAddress->setEnabled(true);
        ui->deviceIdLineEdit->setEnabled(true);
    }
    else
    {
        //nothing for now
    }
}

void MainWindow::enableSendCommands(int state)
{
    //qInfo()<<"Send commands: "<<state;
    if(state==Qt::Checked)
    {
        QString location=ui->commandsLineEdit->text();
        if(!location.isEmpty())
        {
            bool validFile=QFile::exists(location);
            if(validFile)
            {

                bool valid=parseCommands(location);
                if(valid)
                {
                    ui->cmdFileBrowseBtn->setEnabled(false);
                    ui->commandsLineEdit->setEnabled(false);
                    ui->parseCommandsBtn->setEnabled(false);
                    m_commandsFileSelected=true;
                }
                else
                {
                    m_commandsFileSelected=false;
                    ui->cmdFileBrowseBtn->setEnabled(true);
                    ui->commandsLineEdit->setEnabled(true);
                    ui->parseCommandsBtn->setEnabled(true);
                    ui->sendCommandsCheckBox->blockSignals(true);
                    ui->sendCommandsCheckBox->setCheckState(Qt::Unchecked);
                    ui->sendCommandsCheckBox->blockSignals(false);
                }


            }
            else
            {
                m_commandsFileSelected=false;
                ui->cmdFileBrowseBtn->setEnabled(true);
                ui->commandsLineEdit->setEnabled(true);
                ui->parseCommandsBtn->setEnabled(true);
                ui->sendCommandsCheckBox->blockSignals(true);
                ui->sendCommandsCheckBox->setCheckState(Qt::Unchecked);
                ui->sendCommandsCheckBox->blockSignals(false);
                log("Invalid file path");

            }


        }
        else
        {
            ui->cmdFileBrowseBtn->setEnabled(true);
            ui->commandsLineEdit->setEnabled(true);
            ui->parseCommandsBtn->setEnabled(true);
            ui->sendCommandsCheckBox->blockSignals(true);
            ui->sendCommandsCheckBox->setCheckState(Qt::Unchecked);
            ui->sendCommandsCheckBox->blockSignals(false);
            m_commandsFileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->cmdFileBrowseBtn->setEnabled(true);
        ui->commandsLineEdit->setEnabled(true);
        if(!ui->commandsLineEdit->text().isEmpty())
        {
            ui->parseCommandsBtn->setEnabled(true);
        }
        m_commandsFileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}

void MainWindow::commandLineEditTextChanged(const QString &text)
{
    if(text.isEmpty())
    {
        ui->parseCommandsBtn->setEnabled(false);
    }
    else
    {

    }
}

void MainWindow::enableMainCode1Dump(int state)
{
    //qInfo()<<"Main code1 dump: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->mainFirmware1LocationLineEdit->text().isEmpty())
        {
            if(!ui->mainFirmware1StartAddrLineEdit->text().isEmpty())
            {
                bool validFile=QFile::exists(ui->mainFirmware1LocationLineEdit->text());
                if(validFile)
                {
                    ui->mainFirmware1BrowseBtn->setEnabled(false);
                    ui->mainFirmware1LocationLineEdit->setEnabled(false);
                    ui->mainFirmware1StartAddrLineEdit->setEnabled(false);
                    m_mainFirmware1FileSelected=true;

                }
                else
                {
                    m_mainFirmware1FileSelected=false;
                    ui->mainFirmware1BrowseBtn->setEnabled(true);
                    ui->mainFirmware1LocationLineEdit->setEnabled(true);
                    ui->mainFirmware1StartAddrLineEdit->setEnabled(true);
                    ui->mainCodeDump1CheckBox->blockSignals(true);
                    ui->mainCodeDump1CheckBox->setCheckState(Qt::Unchecked);
                    ui->mainCodeDump1CheckBox->blockSignals(false);
                    log("Invalid file path");

                }
            }
            else
            {
                m_mainFirmware1FileSelected=false;
                ui->mainFirmware1BrowseBtn->setEnabled(true);
                ui->mainFirmware1LocationLineEdit->setEnabled(true);
                ui->mainFirmware1StartAddrLineEdit->setEnabled(true);
                ui->mainCodeDump1CheckBox->blockSignals(true);
                ui->mainCodeDump1CheckBox->setCheckState(Qt::Unchecked);
                ui->mainCodeDump1CheckBox->blockSignals(false);
                log("Empty start address");
            }

        }
        else
        {
            ui->mainFirmware1BrowseBtn->setEnabled(true);
            ui->mainFirmware1LocationLineEdit->setEnabled(true);
            ui->mainFirmware1StartAddrLineEdit->setEnabled(true);
            ui->mainCodeDump1CheckBox->blockSignals(true);
            ui->mainCodeDump1CheckBox->setCheckState(Qt::Unchecked);
            ui->mainCodeDump1CheckBox->blockSignals(false);
            m_mainFirmware1FileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->mainFirmware1BrowseBtn->setEnabled(true);
        ui->mainFirmware1LocationLineEdit->setEnabled(true);
        ui->mainFirmware1StartAddrLineEdit->setEnabled(true);
        m_mainFirmware1FileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}

void MainWindow::enableMainCode2Dump(int state)
{
    //qInfo()<<"Main code2 dump: "<<state;
    if(state==Qt::Checked)
    {
        if(!ui->mainFirmware2LocationLineEdit->text().isEmpty())
        {
            if(!ui->mainFirmware2StartAddrLineEdit->text().isEmpty())
            {
                bool validFile=QFile::exists(ui->mainFirmware2LocationLineEdit->text());
                if(validFile)
                {
                    ui->mainFirmware2BrowseBtn->setEnabled(false);
                    ui->mainFirmware2LocationLineEdit->setEnabled(false);
                    ui->mainFirmware2StartAddrLineEdit->setEnabled(false);
                    m_mainFirmware2FileSelected=true;

                }
                else
                {
                    m_mainFirmware2FileSelected=false;
                    ui->mainFirmware2BrowseBtn->setEnabled(true);
                    ui->mainFirmware2LocationLineEdit->setEnabled(true);
                    ui->mainFirmware2StartAddrLineEdit->setEnabled(true);
                    ui->mainCodeDump2CheckBox->blockSignals(true);
                    ui->mainCodeDump2CheckBox->setCheckState(Qt::Unchecked);
                    ui->mainCodeDump2CheckBox->blockSignals(false);
                    log("Invalid file path");

                }
            }
            else
            {
                m_mainFirmware2FileSelected=false;
                ui->mainFirmware2BrowseBtn->setEnabled(true);
                ui->mainFirmware2LocationLineEdit->setEnabled(true);
                ui->mainFirmware2StartAddrLineEdit->setEnabled(true);
                ui->mainCodeDump2CheckBox->blockSignals(true);
                ui->mainCodeDump2CheckBox->setCheckState(Qt::Unchecked);
                ui->mainCodeDump2CheckBox->blockSignals(false);
                log("Empty start address");
            }

        }
        else
        {
            ui->mainFirmware2BrowseBtn->setEnabled(true);
            ui->mainFirmware2LocationLineEdit->setEnabled(true);
            ui->mainFirmware2StartAddrLineEdit->setEnabled(true);
            ui->mainCodeDump2CheckBox->blockSignals(true);
            ui->mainCodeDump2CheckBox->setCheckState(Qt::Unchecked);
            ui->mainCodeDump2CheckBox->blockSignals(false);
            m_mainFirmware2FileSelected=false;
            log("Empty file path");
        }
    }
    else if(state==Qt::Unchecked)
    {
        ui->mainFirmware2BrowseBtn->setEnabled(true);
        ui->mainFirmware2LocationLineEdit->setEnabled(true);
        ui->mainFirmware2StartAddrLineEdit->setEnabled(true);
        m_mainFirmware2FileSelected=false;
    }
    emit enableDisableMCUSelectionSignal();
}



void MainWindow::browseFUSFilePath()
{
    // QString filepath=QFileDialog::getOpenFileName(this,"Select FUS","C:\\Users\\PC\\STM32Cube\\Repository\\STM32Cube_FW_WB_V1.22.1\\Projects\\STM32WB_Copro_Wireless_Binaries\\STM32WB5x",
    //                                                 "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
    //                                                 "All files(*.*)");
    QString filepath=QFileDialog::getOpenFileName(this,"Select FUS",m_fusRedirectPath,
                                                    "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_fusRedirectPath=file.path();
        ui->fusLocationLineEdit->setText(filepath);
        updateFUSStartAddress(filepath);
    }
}

void MainWindow::browseStackFilePath()
{
    // QString filepath=QFileDialog::getOpenFileName(this,"Select Wireless Stack","C:\\Users\\PC\\STM32Cube\\Repository\\STM32Cube_FW_WB_V1.21.0\\Projects\\STM32WB_Copro_Wireless_Binaries\\STM32WB5x",
    //                                                 "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
    //                                                 "All files(*.*)");
    QString filepath=QFileDialog::getOpenFileName(this,"Select Wireless Stack",m_wStackRedirectPath,
                                                    "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_wStackRedirectPath=file.path();
        ui->wStackLineEdit->setText(filepath);
        updateStackStartAddress(filepath);
    }
}

void MainWindow::browseTestFirmwareFilePath()
{
    // QString filepath=QFileDialog::getOpenFileName(this,"Select Test Firmware","D:\\Development\\STM32_IISC\\BLE_HeartRate\\STM32CubeIDE\\Debug",
    //                                                 "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
    //                                                 "All files(*.*)");
    QString filepath=QFileDialog::getOpenFileName(this,"Select Test Firmware",m_testFirmRedirectPath,
                                                    "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_testFirmRedirectPath=file.path();
        ui->testFirmLineEdit->setText(filepath);

    }

}

void MainWindow::browseCommandsFilePath()
{
    QString filepath=QFileDialog::getOpenFileName(this,"Select Commands list file",m_cmdFileRedirectPath,
                                                    "Text files (*.txt);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;
    qInfo()<<"Redirect path: "<<m_cmdFileRedirectPath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_cmdFileRedirectPath=file.path();
        ui->commandsLineEdit->setText(filepath);
        // ui->parseCommandsBtn->setEnabled(true);
    }
    // else
    // {
    //     ui->parseCommandsBtn->setEnabled(false);
    // }
}

void MainWindow::browseResultsFilePath()
{
    // QString filepath=QFileDialog::getSaveFileName(this,"Save results to","D:\\Development\\STM32_IISC\\BLE_HeartRate\\STM32CubeIDE\\Debug",
    //                                                 "PDF files (*.pdf);;"
    //                                                 "All files(*.*)");
    QString filepath=QFileDialog::getExistingDirectory(this,"Select folder for result output",m_resultRedirectPath);
    qInfo()<<"Selected directory: "<<filepath;
    qInfo()<<"Redirect directory: "<<m_resultRedirectPath;
    if(!filepath.isEmpty())
    {
        m_resultRedirectPath=filepath;
        ui->resultsFileLocLineEdit->setText(filepath);
    }
}

void MainWindow::browseMainFirmware1FilePath()
{
    QString filepath=QFileDialog::getOpenFileName(this,"Select Main Firmware",m_mainFirm1RedirectPath,
                                                    "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_mainFirm1RedirectPath=file.path();
        ui->mainFirmware1LocationLineEdit->setText(filepath);
    }
}

void MainWindow::browseMainFirmware2FilePath()
{
    QString filepath=QFileDialog::getOpenFileName(this,"Select Main Firmware",m_mainFirm2RedirectPath,
                                                    "Firmware files (*.bin *.BIN *.hex *.HEX *.elf *.ELF);;"
                                                    "All files(*.*)");
    qInfo()<<"Selected file: "<<filepath;

    if(!filepath.isEmpty())
    {
        QFileInfo file(filepath);
        m_mainFirm2RedirectPath=file.path();
        ui->mainFirmware2LocationLineEdit->setText(filepath);
    }
}

void MainWindow::parseCommandsBtnClicked()
{
    // if(m_commandsFileSelected)
    // {
    QString location=ui->commandsLineEdit->text();
    if(!location.isEmpty())
    {
        parseCommands(location);
        // }
        // else
        // {
        //     log("No file selected");
        // }
    }
    else
    {
        log("Empty file path");
    }


}

bool MainWindow::parseCommands(QString &location)
{
    qInfo()<<"Commands file location: "<<location;
    QString invalidCommand;
    int lineCount;
    bool valid=m_commandParser.parse(location,m_testHeaderInfo,m_testsInfo,invalidCommand,lineCount);

    if(valid)
    {
        log("Commands parsed successfully");
    }
    else
    {
        log(QString("File contains invalid command: ")+invalidCommand);
    }
    return valid;
}

void MainWindow::startAutoBootClicked()
{
    if(!m_bootStarted)
    {
        bool serialPortSelected=!ui->serialPortComboBox->currentText().isEmpty();
        qInfo()<<"Serial port selected: "<<serialPortSelected;
        if(serialPortSelected)
        {
            bool toStart;
            qRegisterMetaType<Parameters_t>("Parameters_t&");
            Parameters_t parameters;
            if(m_dummyRun)
            {
                toStart=true;
                // parameters.dummyRunEnabled=true;
            }
            else
            {
                toStart=m_fusFileSelected | m_stackFileSelected | m_testFirmwareFileSelected | m_commandsFileSelected
                          | m_mainFirmware1FileSelected | m_mainFirmware2FileSelected | m_programID;
            }



            if(toStart)
            {
                log("Start button clicked");

                int currIndex=ui->serialPortComboBox->currentIndex();
                QModelIndex portModelIndex=m_port_model->index(currIndex,0);
                qInfo()<<"Model index: "<<portModelIndex;
                QSerialPortInfo portInfo=m_port_model->getSerialPortInfo(portModelIndex);
                parameters.serialPort=portInfo.portName();
                updateParameters(parameters);
                emit startAutoBoot(true,parameters);
                m_bootStarted=true;
                ui->startAutoBtn->setText("Stop boot");
                parametersEnableDisable(false);
            }
            else
            {
                m_bootStarted=false;
                ui->startAutoBtn->setText("Start boot");
                parametersEnableDisable(true);
            }
        }
        else
        {
            log("No serial port selected");
            m_bootStarted=false;
            ui->startAutoBtn->setText("Start boot");
            parametersEnableDisable(true);
        }
    }
    else
    {
        Parameters_t parameters;
        emit startAutoBoot(false, parameters);
        m_bootStarted=false;
        ui->startAutoBtn->setText("Start boot");
        parametersEnableDisable(true);
    }


}

void MainWindow::mcuSelectComboboxSelectionChanged(const QString &text)
{
    // qInfo()<<"Current text: "<<text;


    if(text=="STM32WB1x")
    {
        //STM32WB1x
        ui->fusUpdateCheckBox->setEnabled(false);
        ui->fusStartAddrLineEdit->setEnabled(false);
        ui->fusStartAddrLineEdit->setEnabled(false);
        ui->fusLocationBrowseBtn->setEnabled(false);
        ui->fusLocationLineEdit->setEnabled(false);
        ui->fusStartAddrLineEdit->setEnabled(false);
        ui->fusLabel->setEnabled(false);

        ui->fusUpdateCheckBox->blockSignals(true);
        ui->fusUpdateCheckBox->setCheckState(Qt::Unchecked);
        ui->fusUpdateCheckBox->blockSignals(false);
        m_fusFileSelected=false;

    }
    // else if(text=="STM32WB5xxx")
    // {
    //     ui->fusUpdateCheckBox->setEnabled(true);
    //     ui->fusStartAddrLineEdit->setEnabled(true);
    //     ui->fusStartAddrLineEdit->setEnabled(true);
    //     ui->fusLocationBrowseBtn->setEnabled(true);
    //     ui->fusLocationLineEdit->setEnabled(true);
    //     ui->fusLabel->setEnabled(true);
    // }
    // else if(text=="Other")
    // {

    //     ui->fusUpdateCheckBox->setEnabled(true);
    //     ui->fusStartAddrLineEdit->setEnabled(true);
    //     ui->fusStartAddrLineEdit->setEnabled(true);
    //     ui->fusLocationBrowseBtn->setEnabled(true);
    //     ui->fusLocationLineEdit->setEnabled(true);
    //     ui->fusLabel->setEnabled(true);
    // }
    // else if(text=="Other")
    // {

    // }
    else
    {
        ui->fusUpdateCheckBox->setEnabled(true);
        ui->fusStartAddrLineEdit->setEnabled(true);
        ui->fusStartAddrLineEdit->setEnabled(true);
        ui->fusLocationBrowseBtn->setEnabled(true);
        ui->fusLocationLineEdit->setEnabled(true);
        ui->fusStartAddrLineEdit->setEnabled(true);
        ui->fusLabel->setEnabled(true);
        updateFUSStartAddress(ui->fusLocationLineEdit->text());
        updateStackStartAddress(ui->wStackLineEdit->text());
    }
}

void MainWindow::enableDisableMCUSelection()
{
    bool toEnable=!(m_fusFileSelected | m_stackFileSelected | m_testFirmwareFileSelected | m_commandsFileSelected
                   | m_mainFirmware1FileSelected | m_mainFirmware2FileSelected | m_dummyRun);

    m_MCUSelectComboBox->setEnabled(toEnable);
}

void MainWindow::dummyRunStateChanged(int state)
{
    if(state==Qt::Checked)
    {
        m_dummyRun=true;
        ui->parametersQFrame->setEnabled(false);
    }
    else if(state==Qt::Unchecked)
    {
        m_dummyRun=false;
        ui->parametersQFrame->setEnabled(true);
    }
    emit enableDisableMCUSelectionSignal();
}

void MainWindow::updateParameters(Parameters_t &parameters)
{
    parameters.dummyRunEnabled=m_dummyRun;
    parameters.toggleBootResetPinEnabled=ui->toggleBootRstCheckBox->isChecked();

    parameters.fusParameter.enabled=ui->fusUpdateCheckBox->isChecked();
    parameters.fusParameter.filePath=ui->fusLocationLineEdit->text();
    parameters.fusParameter.startAddress=ui->fusStartAddrLineEdit->text();

    parameters.wStackParameter.enabled=ui->stackDumpCheckBox->isChecked();
    parameters.wStackParameter.filePath=ui->wStackLineEdit->text();
    parameters.wStackParameter.startAddress=ui->wStackStartAddrLineEdit->text();

    parameters.testFirmParameter.enabled=ui->testCodeDumpCheckBox->isChecked();
    parameters.testFirmParameter.filePath=ui->testFirmLineEdit->text();
    parameters.testFirmParameter.startAddress=ui->testFirmwareStartAddrLineEdit->text();

    parameters.testHeaderInfo=m_testHeaderInfo;
    parameters.testsInfo=m_testsInfo;

    parameters.commandsParameter.enabled=ui->sendCommandsCheckBox->isChecked();
    parameters.commandsParameter.filePath=ui->commandsLineEdit->text();

    parameters.mainFirm1Parameter.enabled=ui->mainCodeDump1CheckBox->isChecked();
    parameters.mainFirm1Parameter.filePath=ui->mainFirmware1LocationLineEdit->text();
    parameters.mainFirm1Parameter.startAddress=ui->mainFirmware1StartAddrLineEdit->text();

    parameters.mainFirm2Parameter.enabled=ui->mainCodeDump2CheckBox->isChecked();
    parameters.mainFirm2Parameter.filePath=ui->mainFirmware2LocationLineEdit->text();
    parameters.mainFirm2Parameter.startAddress=ui->mainFirmware2StartAddrLineEdit->text();

    parameters.deviceId.programDeviceId=ui->programIDCheckBox->isChecked();
    parameters.deviceId.deviceId=ui->deviceIdLineEdit->text();
    // qInfo()
    parameters.deviceId.startAddress=ui->devIdAddress->text();

    parameters.resultParameter.enableResultsExport=ui->saveResultsCheckBox->isChecked();
    parameters.resultParameter.enableLogsExport=ui->saveLogCheckBox->isChecked();
    parameters.resultParameter.resultFilePath=ui->resultsFileLocLineEdit->text();

}

void MainWindow::parametersEnableDisable(bool toEnable)
{
    if(toEnable)
    {
        ui->serialPortComboBox->setEnabled(true);
        ui->scanPushButton->setEnabled(true);
        ui->autoCheckBox->setEnabled(true);
        ui->dummyRunCheckBox->setEnabled(true);
        // ui->toggleBootRstCheckBox->setEnabled(true);
        if(!m_dummyRun)
        {
            ui->parametersQFrame->setEnabled(true);
        }
    }
    else
    {
        ui->serialPortComboBox->setEnabled(false);
        ui->scanPushButton->setEnabled(false);
        ui->autoCheckBox->setEnabled(false);
        ui->dummyRunCheckBox->setEnabled(false);
        // ui->toggleBootRstCheckBox->setEnabled(false);
        ui->parametersQFrame->setEnabled(false);
    }
}

void MainWindow::updateFUSStartAddress(QString filepath)
{
    if(!filepath.isEmpty())
    {
        QString address=getFUSStartAddress(filepath);
        if(!address.isEmpty())
        {
            qInfo()<<"Found Address: "<<address;
            ui->fusStartAddrLineEdit->setText(address);
        }
        else
        {
            qInfo()<<"Address not available";
            ui->fusStartAddrLineEdit->clear();
        }
    }

}

void MainWindow::updateStackStartAddress(QString filepath)
{
    if(!filepath.isEmpty())
    {
        QString address=getStackStartAddress(filepath);
        if(!address.isEmpty())
        {
            qInfo()<<"Found Address: "<<address;
            ui->wStackStartAddrLineEdit->setText(address);
        }
        else
        {
            qInfo()<<"Address not available";
            ui->wStackStartAddrLineEdit->clear();
        }
    }

}

QString MainWindow::getFUSStartAddress(QString path)
{
    QFileInfo filePath(path);
    QString dir=filePath.path();
    // qInfo()<<"File directory: "<<dir;
    // qInfo()<<"File name: "<<filePath.fileName();
    QString releaseFilePath=dir+QDir::separator()+"Release_Notes.html";
    // qInfo()<<"Release file location: "<<releaseFilePath;
    QFile file(releaseFilePath);
    if(file.exists(releaseFilePath))
    {
        // qInfo()<<"Release file exist";

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString html = file.readAll();
            // QString binaryName = "stm32wb1x_BLE_HCI_AdvScan_fw.bin";
            QString binaryName = filePath.fileName();
            QString address = extractFUSAddressFromHtml(html, binaryName);
            // QString addr2=extractAddressForBinaryAndChip(html,binaryName,m_MCUSelectComboBox->currentText());
            // qInfo()<<"Found Address with new fucntion: "<<addr2;
            // if(!address.isEmpty())
            // {
            //     qInfo()<<"Found Address: "<<address;
            //     ui
            // }
            // else
            // {
            //     qInfo()<<"Address not available";
            // }
            return address;

        }
        else
        {
            qInfo()<<"Not able to read Release file";
            return QString();
        }


    }
    else
    {
        qInfo()<<"Release file does not exist";
        return QString();
    }

}

QString MainWindow::getStackStartAddress(QString path)
{
    QFileInfo filePath(path);
    QString dir=filePath.path();
    // qInfo()<<"File directory: "<<dir;
    // qInfo()<<"File name: "<<filePath.fileName();
    QString releaseFilePath=dir+QDir::separator()+"Release_Notes.html";
    // qInfo()<<"Release file location: "<<releaseFilePath;
    QFile file(releaseFilePath);
    if(file.exists(releaseFilePath))
    {
        // qInfo()<<"Release file exist";

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString html = file.readAll();
            // QString binaryName = "stm32wb1x_BLE_HCI_AdvScan_fw.bin";
            QString binaryName = filePath.fileName();
            QString address = extractStackAddressFromHtml(html, binaryName);
            // QString addr2=extractAddressForBinaryAndChip(html,binaryName,m_MCUSelectComboBox->currentText());
            // qInfo()<<"Found Address with new fucntion: "<<addr2;
            // if(!address.isEmpty())
            // {
            //     qInfo()<<"Found Address: "<<address;
            //     ui
            // }
            // else
            // {
            //     qInfo()<<"Address not available";
            // }
            return address;

        }
        else
        {
            qInfo()<<"Not able to read Release file";
            return QString();
        }


    }
    else
    {
        qInfo()<<"Release file does not exist";
        return QString();
    }

}

QString MainWindow::extractFUSAddressFromHtml(const QString &html, const QString &binaryName)
{
    QRegularExpression regex(
        QString("<tr[^>]*>\\s*<td>%1</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)(?:<[^>]+>)*([0-9A-Fa-f]+)</td>")
            .arg(QRegularExpression::escape(binaryName))
        );
    // QString str=QString("<tr[^>]*>\\s*<td>%1</td>\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>")
    //         .arg(QRegularExpression::escape(binaryName));
    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch())
    {

        if(m_MCUSelectComboBox->currentText()!="STM32WB1x")
        {
            int index=m_MCUSelectComboBox->currentIndex();
            QString str1=match.captured(index*3+1);
            QString str2=match.captured(index*3+2);
            QString str3=match.captured(index*3+3);
            return (str1+str2+str3);
        }
    }
    return {};
}
QString MainWindow::extractStackAddressFromHtml(const QString &html, const QString &binaryName)
{
    QRegularExpression regex(
        QString("<tr[^>]*>\\s*<td>%1</td>\\s*<td>(0x[0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
                "\\s*<td>(0x[0-9A-Fa-f]+)</td>")
            .arg(QRegularExpression::escape(binaryName))
        );
    // QString str=QString("<tr[^>]*>\\s*<td>%1</td>\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>"
    //             "\\s*<td>(0x[0-9A-Fa-f]+)</td>")
    //         .arg(QRegularExpression::escape(binaryName));
    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch())
    {

        if(m_MCUSelectComboBox->currentText()=="STM32WB1x")
        {
            return match.captured(1);
        }
        else
        {
            return match.captured(m_MCUSelectComboBox->currentIndex()+1);
        }

    }
    return {};
}
QString MainWindow::extractAddressForBinaryAndChip(const QString &html, const QString &binaryName, const QString &chipName)
{
    // Static regexes to avoid recompilation and satisfy Clazy
    static const QRegularExpression headerRegex(
        R"(<tr class="header">(.*?)</tr>)",
        QRegularExpression::DotMatchesEverythingOption
        );

    static const QRegularExpression thRegex(R"(<th[^>]*>(.*?)</th>)");
    static const QRegularExpression rowRegex(
        R"(<tr[^>]*>\s*<td>([^<]+)</td>(.*?)</tr>)",
        QRegularExpression::DotMatchesEverythingOption
        );

    static const QRegularExpression tdRegex(R"(<td[^>]*>(.*?)</td>)");

    // Step 1: Find the header row
    QRegularExpressionMatch headerMatch = headerRegex.match(html);
    if (!headerMatch.hasMatch())
        return {};

    QString headerRow = headerMatch.captured(1);
    QRegularExpressionMatchIterator headerIter = thRegex.globalMatch(headerRow);

    int chipColumnIndex = -1;
    int columnIndex = 0;
    while (headerIter.hasNext()) {
        QString columnName = headerIter.next().captured(1).trimmed();
        if (columnName.contains(chipName, Qt::CaseInsensitive)) {
            chipColumnIndex = columnIndex;
            break;
        }
        ++columnIndex;
    }

    if (chipColumnIndex == -1)
        return {}; // Chip column not found

    // Step 2: Match rows and find binary
    QRegularExpressionMatchIterator rowIter = rowRegex.globalMatch(html);
    while (rowIter.hasNext()) {
        QRegularExpressionMatch match = rowIter.next();
        QString binName = match.captured(1).trimmed();
        if (binName == binaryName) {
            QString cells = match.captured(2);
            QRegularExpressionMatchIterator cellIter = tdRegex.globalMatch(cells);

            int i = 0;
            while (cellIter.hasNext()) {
                QString cellText = cellIter.next().captured(1).trimmed();
                if (i == chipColumnIndex - 1) { // -1 for first <td> already captured as binaryName
                    return cellText;
                }
                ++i;
            }
        }
    }

    return {}; // Not found
}

void MainWindow::testTimerTimeout()
{
    m_time+=5;
    if(m_time>100)
    {
        log("Next");
        m_testTimer.stop();
        m_time=0;
    }
    else
    {
        qInfo()<<"Time value: "<<m_time;
        QString progress=QString("Progress: ")+QString::number(m_time)+"%";
        updateLog(progress);
    }

}



void MainWindow::closeEvent(QCloseEvent *event)
{
    // qInfo()<<"App closed: "<<event;
    if(m_saveLog)
    {
        // ui->logDisplayPlainTextEdit->toPlainText()
    }
    saveAllSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveAllSettings()
{
    QSettings settings(PROGRAM_NAME, PROGRAM_NAME);
    settings.beginGroup(SettingGroup_Parameters);


    settings.setValue(SG_DummyRunEnabled,ui->dummyRunCheckBox->isChecked());
    settings.setValue(SG_FusLocation,ui->fusLocationLineEdit->text());
    settings.setValue(SG_FusStartAddress,ui->fusStartAddrLineEdit->text());
    settings.setValue(SG_WstackLocation,ui->wStackLineEdit->text());
    settings.setValue(SG_WstackAddress,ui->wStackStartAddrLineEdit->text());
    settings.setValue(SG_TestFirmLocation,ui->testFirmLineEdit->text());
    settings.setValue(SG_TestFirmAddress,ui->testFirmwareStartAddrLineEdit->text());
    settings.setValue(SG_CommandsLocation,ui->commandsLineEdit->text());
    settings.setValue(SG_MainFirm1Location,ui->mainFirmware1LocationLineEdit->text());
    settings.setValue(SG_MainFirm1Address,ui->mainFirmware1StartAddrLineEdit->text());
    settings.setValue(SG_MainFirm2Location,ui->mainFirmware2LocationLineEdit->text());
    settings.setValue(SG_MainFirm2Address,ui->mainFirmware2StartAddrLineEdit->text());
    settings.setValue(SG_DeviceIdValue,ui->deviceIdLineEdit->text());
    settings.setValue(SG_DeviceIdAddress,ui->devIdAddress->text());
    settings.setValue(SG_ResultDirectory,ui->resultsFileLocLineEdit->text());

    settings.endGroup();
}

void MainWindow::loadAllSettings()
{
    QSettings settings(PROGRAM_NAME, PROGRAM_NAME);
    settings.beginGroup(SettingGroup_Parameters);


    settings.value(SG_DummyRunEnabled,false);

    QString defaultPath=QDir::homePath()+ QDir::separator()+"STM32Cube"+QDir::separator()+"Repository";
    QString fusPath=settings.value(SG_FusLocation,QString()).toString();
    if(fusPath.isEmpty())
    {
        m_fusRedirectPath=defaultPath;
    }
    else
    {
        m_fusRedirectPath=fusPath;
    }

    ui->fusLocationLineEdit->setText(fusPath);
    ui->fusStartAddrLineEdit->setText(settings.value(SG_FusStartAddress,QString()).toString());

    QString wstackPath=settings.value(SG_WstackLocation,QString()).toString();
    if(wstackPath.isEmpty())
    {
        m_wStackRedirectPath=defaultPath;
    }
    else
    {
        m_wStackRedirectPath=wstackPath;
    }
    ui->wStackLineEdit->setText(wstackPath);
    ui->wStackStartAddrLineEdit->setText(settings.value(SG_WstackAddress,QString()).toString());

    QString testFirmPath=settings.value(SG_TestFirmLocation,QString()).toString();
    m_testFirmRedirectPath=testFirmPath;
    ui->testFirmLineEdit->setText(testFirmPath);
    ui->testFirmwareStartAddrLineEdit->setText(settings.value(SG_TestFirmAddress,"0x08000000").toString());

    QString cmdPath=settings.value(SG_CommandsLocation,QString()).toString();
    m_cmdFileRedirectPath=cmdPath;
    ui->commandsLineEdit->setText(cmdPath);

    QString firm1Path=settings.value(SG_MainFirm1Location,QString()).toString();
    m_mainFirm1RedirectPath=firm1Path;
    ui->mainFirmware1LocationLineEdit->setText(firm1Path);
    ui->mainFirmware1StartAddrLineEdit->setText(settings.value(SG_MainFirm1Address,"0x08000000").toString());

    QString firm2Path=settings.value(SG_MainFirm2Location,QString()).toString();
    m_mainFirm2RedirectPath=firm2Path;
    ui->mainFirmware2LocationLineEdit->setText(firm2Path);
    ui->mainFirmware2StartAddrLineEdit->setText(settings.value(SG_MainFirm2Address,"0x08007000").toString());
    ui->deviceIdLineEdit->setText(settings.value(SG_DeviceIdValue,QString()).toString());
    ui->devIdAddress->setText(settings.value(SG_DeviceIdAddress,QString()).toString());

    QString resultPath=settings.value(SG_ResultDirectory,QString()).toString();
    m_resultRedirectPath=resultPath;
    ui->resultsFileLocLineEdit->setText(resultPath);

    settings.endGroup();
}

