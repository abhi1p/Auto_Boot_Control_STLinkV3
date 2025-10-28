#ifndef PARAMETERS_H
#define PARAMETERS_H

// #include "port_model.h"
#include <QObject>
#include <QSerialPortInfo>
#include <QString>
#include <QMap>

const char SettingGroup_Parameters[]="Parameters";
const char SG_DummyRunEnabled[]="DummyRunEnabled";
const char SG_FusLocation[]="FusLocation";
const char SG_FusStartAddress[]="FusStartAddress";
const char SG_WstackLocation[]="WstackLocation";
const char SG_WstackAddress[]="WstackAddress";
const char SG_TestFirmLocation[]="TestFirmLocation";
const char SG_TestFirmAddress[]="TestFirmAddress";
const char SG_CommandsLocation[]="CommandsLocation";
const char SG_MainFirm1Location[]="MainFirm1Location";
const char SG_MainFirm1Address[]="MainFirm1Address";
const char SG_MainFirm2Location[]="MainFirm2Location";
const char SG_MainFirm2Address[]="MainFirm2Address";
const char SG_DeviceIdValue[]="DeviceIdValue";
const char SG_DeviceIdAddress[]="DeviceIdAddress";
const char SG_ResultDirectory[]="ResultDirectory";

typedef QMap<QString,QString> Test_Header_t;

// typedef struct
// {
//     bool toPrint;
//     QString command;
//     QString commandValue;
//     QString yResponse;
//     QString nResponse;
//     QString testDescription;
//     QString testReqirement;
// }Test_Info_t;
typedef struct
{
    bool toPrint;
    QString command;
    // QString commandValue;
    QString firstParam;
    QString secondParam;
    QString thirdParam;
    QString fourthParam;
    QString testDescription;
    QString testReqirement;
}Test_Info_t;

typedef struct
{
    QString testDescription;
    QString status;
    QString testReqirement;
}Test_Output_t;

typedef struct
{
    Test_Header_t header;
    QList<Test_Output_t> testsTable;
}PDF_t;

typedef struct
{
    bool enabled;
    QString filePath;
    QString startAddress;
}Parameter_t;

typedef struct
{
    bool enableLogsExport;
    bool enableResultsExport;
    QString resultFilePath;
}Result_t;

typedef struct
{
    bool programDeviceId;
    QString deviceId;
    QString startAddress;
} DeviceId_t;

typedef struct
{
    QString serialPort;
    bool dummyRunEnabled;
    bool toggleBootResetPinEnabled;
    Parameter_t fusParameter;
    Parameter_t wStackParameter;
    Parameter_t testFirmParameter;
    Test_Header_t testHeaderInfo;
    QList<Test_Info_t> testsInfo;
    Parameter_t commandsParameter;
    Parameter_t mainFirm1Parameter;
    Parameter_t mainFirm2Parameter;
    Result_t resultParameter;
    DeviceId_t deviceId;
}Parameters_t;

//qRegisterMetaType<Parameters_t>("Parameters_t");
Q_DECLARE_METATYPE(Parameters_t)
#endif // PARAMETERS_H
