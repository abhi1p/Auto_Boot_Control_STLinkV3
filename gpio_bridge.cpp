#include "gpio_bridge.h"

GPIO_Bridge::GPIO_Bridge(QObject *parent)
    : QObject{parent}
{

    for (int i=0; i<SERIAL_NUM_STR_MAX_LEN; i++) {
        m_serialNumber[i] = 0;
    }
    connect(&m_timer,&QTimer::timeout,this,&GPIO_Bridge::readGPIO,Qt::QueuedConnection);
    // GPIOBridgeInit();
}

GPIO_Bridge::~GPIO_Bridge()
{
    DisconnectToBridge();
}

bool GPIO_Bridge::GPIOBridgeInit(QMap<QString, int> pinMap)
{
    //Brg* pBrg = NULL;
    //STLinkInterface *m_pStlinkIf = NULL;

    STLinkIf_StatusT ifStat = STLINKIF_NO_ERR;


    // In case previously used, close the previous connection (not the case here)
    if (!m_pBrg.isNull())
    {
        m_pBrg->CloseBridge(COM_UNDEF_ALL);
        m_pBrg->CloseStlink();
        m_pBrg.clear();
    }
    if (!m_pStlinkIf.isNull())
    {// never delete STLinkInterface before Brg that is using it.
        m_pStlinkIf.clear();
    }

    // Create USB BRIDGE interface
    m_pStlinkIf.reset(new STLinkInterface(STLINK_BRIDGE));
    ifStat = m_pStlinkIf->LoadStlinkLibrary2(m_dllPath);
    if( ifStat!=STLINKIF_NO_ERR )
    {
        qInfo("STLinkUSBDriver library (dll) issue \n");
    }
    else
    {
        qInfo()<<"STLinkUSBDriver library (dll) loaded";
    }

    bool status=connectToBridge(pinMap);

    return status;


}

bool GPIO_Bridge::connectToBridge(QMap<QString, int> pinMap)
{
    Brg_StatusT brgStat = BRG_NO_ERR;
    int firstDevNotInUse=-1;

    brgStat = SelectSTLink(m_pStlinkIf.data(), &firstDevNotInUse);

    // USB Connection to a given device done with Brg
    if (brgStat == BRG_NO_ERR)
    {
        m_pBrg.reset(new Brg(*m_pStlinkIf.data()));
        brgStat = Connect(firstDevNotInUse);// Connect to the selected STLink
    }
    if(brgStat == BRG_NO_ERR)
    {
        qInfo()<<"Connected to stlink bridge";
        emit connectedToBridge(true);

        // if(GPIOInitTest()== BRG_NO_ERR)
        // {

        //     m_timer.setInterval(2000);
        //     m_timer.start();
        //     qInfo()<<"Read GPIO timer started";
        // }
        brgStat=GPIO_Init(pinMap);
        if(brgStat== BRG_NO_ERR)
        {

            // m_timer.setInterval(2000);
            // m_timer.start();
            // qInfo()<<"Read GPIO timer started";
            return true;
        }
        else
        {
            return false;
        }

    }
    else
    {
        //qInfo()<<"Connection error: "<<brgStat;
        emit connectedToBridge(false);
        return false;
    }
    // log(brgStat);
}

void GPIO_Bridge::log(Brg_StatusT status)
{
    switch(status)
    {

        case BRG_NO_ERR:
        {
            qInfo()<<"No error";
            break;
        }
        case BRG_CONNECT_ERR:
        {

            break;
        }
        case BRG_DLL_ERR:
        {

            break;
        }
        case BRG_USB_COMM_ERR:
        {
            qInfo()<<"USB Communication error";
            break;
        }
        case BRG_NO_DEVICE:
        {

            break;
        }
        case BRG_OLD_FIRMWARE_WARNING:
        {

            break;
        }
        case BRG_TARGET_CMD_ERR:
        {

            break;
        }
        case BRG_PARAM_ERR:
        {

            break;
        }
        case BRG_CMD_NOT_SUPPORTED:
        {

            break;
        }
        case BRG_GET_INFO_ERR:
        {

            break;
        }
        case BRG_STLINK_SN_NOT_FOUND:
        {

            break;
        }
        case BRG_NO_STLINK:
        {

            break;
        }
        case BRG_NOT_SUPPORTED:
        {

            break;
        }
        case BRG_PERMISSION_ERR:
        {

            break;
        }
        case BRG_ENUM_ERR:
        {

            break;
        }
        case BRG_COM_FREQ_MODIFIED:
        {

            break;
        }
        case BRG_COM_FREQ_NOT_SUPPORTED:
        {

            break;
        }
        case BRG_SPI_ERR:
        {

            break;
        }
        case BRG_I2C_ERR:
        {

            break;
        }
        case BRG_CAN_ERR:
        {

            break;
        }
        case BRG_TARGET_CMD_TIMEOUT:
        {

            break;
        }
        case BRG_COM_INIT_NOT_DONE:
        {

            break;
        }
        case BRG_COM_CMD_ORDER_ERR:
        {

            break;
        }
        case BRG_BL_NACK_ERR:
        {

            break;
        }
        case BRG_VERIF_ERR:
        {

            break;
        }
        case BRG_MEM_ALLOC_ERR:
        {

            break;
        }
        case BRG_GPIO_ERR:
        {

            break;
        }
        case BRG_OVERRUN_ERR:
        {

            break;
        }
        case BRG_CMD_BUSY:
        {

            break;
        }
        case BRG_CLOSE_ERR:
        {

            break;
        }
        case BRG_INTERFACE_ERR:
        {

            break;
        }
        case BRG_CMD_NOT_ALLOWED:
        {

            break;
        }

    }
}

/*****************************************************************************/
// STLINK USB management
/*****************************************************************************/
Brg_StatusT GPIO_Bridge::SelectSTLink(STLinkInterface* pStlinkIf, int* pFirstDevNotInUse)
{
    uint32_t i, numDevices;
    TDeviceInfo2 devInfo2;
    STLinkIf_StatusT ifStatus = STLINKIF_NO_ERR;
    STLink_EnumStlinkInterfaceT stlinkIfId;

    if ((pStlinkIf == NULL) || (pFirstDevNotInUse == NULL))
    {
        qInfo("Internal parameter error in SelectSTLink\n");
        return BRG_PARAM_ERR;
    }
    stlinkIfId = pStlinkIf->GetIfId();
    if (stlinkIfId != STLINK_BRIDGE)
    {
        qInfo("Wrong interface in SelectSTLink\n");
        return BRG_PARAM_ERR;
    }

    ifStatus = pStlinkIf->EnumDevices(&numDevices, FALSE);
    // Choose the first STLink Bridge available
    if ((ifStatus == STLINKIF_NO_ERR) || (ifStatus == STLINKIF_PERMISSION_ERR))
    {
        qInfo("%d BRIDGE device found\n", (int)numDevices);

        for( i=0; i<numDevices; i++ )
        {
            ifStatus = pStlinkIf->GetDeviceInfo2(i, &devInfo2, sizeof(devInfo2));
            qInfo("Bridge %d PID: 0X%04hx SN:%s\n", (int)i, (unsigned short)devInfo2.ProductId, devInfo2.EnumUniqueId);

            if( (*pFirstDevNotInUse==-1) && (devInfo2.DeviceUsed == false) )
            {
                *pFirstDevNotInUse = i;
                memcpy(m_serialNumber, &devInfo2.EnumUniqueId, SERIAL_NUM_STR_MAX_LEN);
                qInfo(" SELECTED BRIDGE Stlink SN:%s\n", m_serialNumber);
            }
        }
    } else if (ifStatus == STLINKIF_CONNECT_ERR)
    {
        qInfo("No STLink BRIDGE device detected\n");
    } else
    {
        qInfo("Enum error (status = %d)\n", ifStatus);
        if (ifStatus == STLINKIF_NO_STLINK)
        {
            qInfo("No BRIDGE STLink available\n");
        }
    }

    return Brg::ConvSTLinkIfToBrgStatus(ifStatus);
}

Brg_StatusT GPIO_Bridge::Connect(int deviceNb)
{
    // The firmware may not be the very last one, but it may be OK like that (just inform)
    bool bOldFirmwareWarning=false;
    Brg_StatusT brgStat = BRG_NO_ERR;
    if (m_pBrg.isNull()) {
        return BRG_CONNECT_ERR;
    }
    //m_pBrg = pBrg;
    // Open the STLink connection
    if (brgStat == BRG_NO_ERR)
    {
        m_pBrg->SetOpenModeExclusive(true);

        brgStat = m_pBrg->OpenStlink(deviceNb);

        if (brgStat == BRG_NOT_SUPPORTED) {
            qInfo("BRIDGE not supported SN:%s\n", m_serialNumber);
        }
        if (brgStat == BRG_OLD_FIRMWARE_WARNING) {
            // Status to restore at the end if all is OK
            bOldFirmwareWarning = true;
            brgStat = BRG_NO_ERR;
        }
    }

    // Test Voltage command
    if (brgStat == BRG_NO_ERR)
    {
        float voltage = 0;
        // T_VCC pin must be connected to target voltage on bridge or debug connector
        // T_VCC input is mandatory for STLink using levelshifter (STLINK-V3PWR or STLINK-V3SET+B-STLINK-VOLT/ISOL),
        // else bridge signals are all 0
        brgStat = m_pBrg->GetTargetVoltage(&voltage);
        if (brgStat != BRG_NO_ERR)
        {
            qInfo("BRIDGE get voltage error \n");
        } else if (voltage < 1)
        {
            qInfo("BRIDGE get voltage WARNING: %fV < 1V, check if T_VCC pin is connected to target voltage on bridge (or debug) connector \n", (double)voltage);
        } else
        {
            qInfo("BRIDGE get voltage: %f V \n", (double)voltage);
        }
    }

    if ((brgStat == BRG_NO_ERR) && (bOldFirmwareWarning == true))
    {
        // brgStat = BRG_OLD_FIRMWARE_WARNING;
        qInfo("BRG_OLD_FIRMWARE_WARNING: v%d B%d \n",(int)m_pBrg->m_Version.Major_Ver, (int)m_pBrg->m_Version.Bridge_Ver);
    }

    return brgStat;

}

Brg_StatusT GPIO_Bridge::GPIO_Init(QMap<QString, int> pinMap)
{
    Brg_StatusT BrgStatus = BRG_NO_ERR;
    Brg_GpioInitT gpioParams;
    Brg_GpioConfT gpioConf[BRG_GPIO_MAX_NB];
    uint8_t gpioMsk=0;


    //Pin init
    gpioMsk = BRG_GPIO_ALL;
    gpioParams.GpioMask = gpioMsk; // BRG_GPIO_0 1 2 3
    gpioParams.ConfigNb = BRG_GPIO_MAX_NB; //must be BRG_GPIO_MAX_NB or 1 (if 1 then pGpioConf[0] is used for all gpios)
    gpioParams.pGpioConf = &gpioConf[0];

    //Reset pin init
    gpioConf[pinMap.value("Reset")].Mode = GPIO_MODE_OUTPUT; // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
    gpioConf[pinMap.value("Reset")].Speed = GPIO_SPEED_LOW; // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
    gpioConf[pinMap.value("Reset")].Pull = GPIO_NO_PULL; // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
    gpioConf[pinMap.value("Reset")].OutputType = GPIO_OUTPUT_PUSHPULL; // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN

    //Boot pin init
    gpioConf[pinMap.value("Boot")].Mode = GPIO_MODE_OUTPUT; // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
    gpioConf[pinMap.value("Boot")].Speed = GPIO_SPEED_LOW; // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
    gpioConf[pinMap.value("Boot")].Pull = GPIO_NO_PULL; // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
    gpioConf[pinMap.value("Boot")].OutputType = GPIO_OUTPUT_PUSHPULL; // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN

    //Connect pin init
    gpioConf[pinMap.value("Connect")].Mode = GPIO_MODE_INPUT; // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
    gpioConf[pinMap.value("Connect")].Speed = GPIO_SPEED_LOW; // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
    gpioConf[pinMap.value("Connect")].Pull = GPIO_PULL_DOWN; // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
    gpioConf[pinMap.value("Connect")].OutputType = GPIO_OUTPUT_PUSHPULL; // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN

    //NC pin init
    gpioConf[pinMap.value("NC")].Mode = GPIO_MODE_ANALOG; // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
    gpioConf[pinMap.value("NC")].Speed = GPIO_SPEED_LOW; // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
    gpioConf[pinMap.value("NC")].Pull = GPIO_NO_PULL; // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
    gpioConf[pinMap.value("NC")].OutputType = GPIO_OUTPUT_PUSHPULL; // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN


    BrgStatus = m_pBrg->InitGPIO(&gpioParams);

    //writePin(0,GPIO_SET);

    return BrgStatus;

}

Brg_StatusT GPIO_Bridge::GPIOInitTest()
{
    Brg_StatusT BrgStatus = BRG_NO_ERR;
    Brg_GpioInitT gpioParams;
    Brg_GpioConfT gpioConf[BRG_GPIO_MAX_NB];
    uint8_t gpioMsk=0;

    int i;
    gpioMsk = BRG_GPIO_ALL;
    gpioParams.GpioMask = gpioMsk; // BRG_GPIO_0 1 2 3
    gpioParams.ConfigNb = BRG_GPIO_MAX_NB; //must be BRG_GPIO_MAX_NB or 1 (if 1 then pGpioConf[0] is used for all gpios)
    gpioParams.pGpioConf = &gpioConf[0];
    for(i=0; i<BRG_GPIO_MAX_NB; i++) {
        gpioConf[i].Mode = GPIO_MODE_INPUT; // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
        gpioConf[i].Speed = GPIO_SPEED_LOW; // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
        gpioConf[i].Pull = GPIO_NO_PULL; // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
        gpioConf[i].OutputType = GPIO_OUTPUT_PUSHPULL; // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN
    }
    BrgStatus = m_pBrg->InitGPIO(&gpioParams);
    if( BrgStatus != BRG_NO_ERR ) {
        printf("Bridge Gpio init failed (mask=%d, gpio0: mode= %d, pull = %d, ...)\n",(int)gpioParams.GpioMask, (int)gpioConf[0].Mode, (int)gpioConf[0].Pull);
    }
    return BrgStatus;
}

void GPIO_Bridge::readGPIO()
{
    Brg_StatusT BrgStatus = BRG_NO_ERR;
    Brg_GpioValT gpioReadVal[BRG_GPIO_MAX_NB];
    Brg_GpioValT gpioWriteVal[BRG_GPIO_MAX_NB];
    uint8_t gpioMsk=BRG_GPIO_ALL, gpioErrMsk;
    float voltage;
    m_pBrg->GetTargetVoltage(&voltage);
    BrgStatus = m_pBrg->ReadGPIO(gpioMsk, &gpioReadVal[0], &gpioErrMsk);
    if( (BrgStatus != BRG_NO_ERR) || (gpioErrMsk!=0) )
    {
        qInfo(" Bridge Read error\n");
    }
    else
    {
        for(int i=0; i<BRG_GPIO_MAX_NB; i++)
        {
            qInfo()<<"GPIO"<<i<<": "<<gpioReadVal[i];
        }
        qInfo()<<"GPIO read completed";
    }
}

Brg_GpioValT GPIO_Bridge::readPin(int pinNo)
{
    Brg_StatusT BrgStatus = BRG_NO_ERR;
    Brg_GpioValT gpioReadVal[BRG_GPIO_MAX_NB];
    uint8_t gpioMsk=BRG_GPIO_ALL, gpioErrMsk;
    BrgStatus = m_pBrg->ReadGPIO(gpioMsk, &gpioReadVal[0], &gpioErrMsk);
    // qInfo()<<"Pin "<<pinNo<<" value is "<<gpioReadVal[pinNo];
    if(BrgStatus==BRG_NO_ERR)
    {
        return gpioReadVal[pinNo];
        // return false;
    }
    else
    {
        return GPIO_RESET;
    }

}

void GPIO_Bridge::readAllPin(Brg_GpioValT *value)
{

    uint8_t gpioMsk=BRG_GPIO_ALL, gpioErrMsk;
    m_pBrg->ReadGPIO(gpioMsk, value, &gpioErrMsk);
}

void GPIO_Bridge::writePin(int pinNo, Brg_GpioValT value)
{
    // Brg_StatusT BrgStatus = BRG_NO_ERR;
    // Brg_GpioValT gpioWriteVal=(Brg_GpioValT)value;
    Brg_GpioValT gpioVal[BRG_GPIO_MAX_NB];
    readAllPin(&gpioVal[0]);
    gpioVal[pinNo]=value;
    uint8_t gpioMsk=BRG_GPIO_ALL, gpioErrMsk;
    m_pBrg->SetResetGPIO(gpioMsk,&gpioVal[0],&gpioErrMsk);
}

bool GPIO_Bridge::validVcc()
{
    float voltage;
    Brg_StatusT brgStat = m_pBrg->GetTargetVoltage(&voltage);
    // qInfo()<<"Target voltage: "<<voltage;
    if(brgStat==BRG_NO_ERR)
    {
        if((voltage>MIN_VALID_VCC) && (voltage<MAX_VALID_VCC))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void GPIO_Bridge::DisconnectToBridge()
{
    if (!m_pBrg.isNull())
    {
        //robustness in case not already done
         Brg_StatusT brgStat = m_pBrg->CloseBridge(COM_UNDEF_ALL);
        qInfo()<<"Bridge closed";
        log(brgStat);

         // if(brgStat)
        // break link to current STLink BRIDGE


        m_pBrg.clear();
    }
    m_timer.stop();
}
