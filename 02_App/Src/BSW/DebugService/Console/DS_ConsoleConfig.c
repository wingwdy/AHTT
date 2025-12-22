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
    uint8_t port = atoi(argv[2]);

    if (argc == 3)
    {
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


DSCONSOLE_CFG_ADD_CMD(reboot,    DSConsoleCfg_Reboot, "reboot" reboot system);
DSCONSOLE_CFG_ADD_CMD(charge,    DSConsoleCfg_ChargeCtrl, "charge start/stop 0/1" start/stop charge);
DSCONSOLE_CFG_ADD_CMD(showStack, DSConsoleCfg_ShowStack, "showStack" show stack info);














