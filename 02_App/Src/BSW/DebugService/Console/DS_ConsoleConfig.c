/******************************************************************************
* File Name          : DS_ConsoleConfig.c
* Description        : Code for Serial console debugging
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      chenls    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "DS_ConsoleConfig.h"

#include "Asw_Charge.h"
#include "Mcal_Mcu.h"
#include "PortTask.h"
#include "Cdd_ModeM.h"
/************************s*******************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/



/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static int32_t DSConsoleCfg_Reboot(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ChargeCtrl(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ShowStack(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_EnterFactoryMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_HandleGbMode(int32_t argc, char *argv[]);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static int32_t DSConsoleCfg_Reboot(int32_t argc, char *argv[])
{
    McalMcu_SystemReset();
    return 0;
} 

static int32_t DSConsoleCfg_ChargeCtrl(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint8_t port = 0;

    if (argc == 3)
    {
        port = atoi(argv[2]);

        if (0 == strcmp(argv[1], "start"))
        {
            AswCharge_StartAuth(port);
        }
        else if (0 == strcmp(argv[1], "stop"))
        {
            AswCharge_StopAuth(port);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static int32_t DSConsoleCfg_ShowStack(int32_t argc, char *argv[])
{
    portTask_ShowStackInfo();
    return 0;
}

static int32_t DSConsoleCfg_EnterFactoryMode(int32_t argc, char *argv[])
{
    CddModeM_EnterFactoryMode();
    return 0;
}

static int32_t DSConsoleCfg_HandleGbMode(int32_t argc, char *argv[])
{
    uint8_t mode = 0;

    if (argc == 2)
    {
        mode = atoi(argv[1]);

        if (mode == 1)
        {
            CddModeM_EnterGBMode();
        }
        else if (mode == 2)
        {
            CddModeM_ExitGBMode();
        }
        else
        {}
    }

    return 0;
}

DSCONSOLE_CFG_ADD_CMD(reboot,        DSConsoleCfg_Reboot, "reboot" reboot system);
DSCONSOLE_CFG_ADD_CMD(charge,        DSConsoleCfg_ChargeCtrl, "charge start/stop 0/1" start/stop charge);
DSCONSOLE_CFG_ADD_CMD(showStack,     DSConsoleCfg_ShowStack, "showStack" show stack info);
DSCONSOLE_CFG_ADD_CMD(testmode,      DSConsoleCfg_EnterFactoryMode, "testmode" Enter factoryMode);
DSCONSOLE_CFG_ADD_CMD(gbmode,        DSConsoleCfg_HandleGbMode, "gbmode 1 / 2" EnterGbMode/ExsitGbMode);











