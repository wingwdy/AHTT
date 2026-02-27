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
#include "Asw_PlatM.h"
#include "Asw_ErrorHandle.h"
#include "Asw_Monitor.h"

#include "Cdd_ModeM.h"
#include "Cdd_NetM.h"

#include "SS_Snapshot.h"
#include "SS_Ucm.h"

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
static int32_t DSConsoleCfg_EnterFactoryMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ExsistFactoryMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_HandleGbMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_SetPara(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_GetPara(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ReadAgingState(int32_t argc, char *argv[]);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/
DSCONSOLE_CFG_ADD_CMD(reboot,        DSConsoleCfg_Reboot, "reboot" reboot system);
DSCONSOLE_CFG_ADD_CMD(charge,        DSConsoleCfg_ChargeCtrl, "charge start/stop 0/1" start/stop charge);
DSCONSOLE_CFG_ADD_CMD(showStack,     DSConsoleCfg_ShowStack, "showStack" show stack info);
DSCONSOLE_CFG_ADD_CMD(testmode,      DSConsoleCfg_EnterFactoryMode, "testmode" Enter factoryMode);
DSCONSOLE_CFG_ADD_CMD(workmode,      DSConsoleCfg_ExsistFactoryMode, "workmode" Exsist factoryMode);
DSCONSOLE_CFG_ADD_CMD(gbmode,        DSConsoleCfg_HandleGbMode, "gbmode 1 / 2" EnterGbMode/ExsitGbMode);
DSCONSOLE_CFG_ADD_CMD(set,           DSConsoleCfg_SetPara, "set xxx" set param);
DSCONSOLE_CFG_ADD_CMD(get,           DSConsoleCfg_GetPara, "get xxx" get param);
DSCONSOLE_CFG_ADD_CMD(ReadAgingState,DSConsoleCfg_ReadAgingState, "ReadAgingState" ReadAgingState);

static int32_t DSConsoleCfg_Reboot(int32_t argc, char *argv[])
{
    McalMcu_SystemReset();
    return 0;
} 

static int32_t DSConsoleCfg_ChargeCtrl(int32_t argc, char *argv[])
{
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
            AswErrhandle_SetErrExsitCallback(port, eSrc_MannulStop);
        }
        else
        {}
    }

    return 0;
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

static int32_t DSConsoleCfg_ExsistFactoryMode(int32_t argc, char *argv[])
{
    CddModeM_ExsitFactoryMode();
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

static int32_t DSConsoleCfg_GetPara(int32_t argc, char *argv[])
{
    uint8_t *pTemp = NULL;
    uint8_t setResult = FALSE;

    if (argc == 2)
    { 
        if (0 == strcmp(argv[1], "chargeInfo"))
        {
            AswMonitor_PrintChargeData();
        }
        else if (0 == strcmp(argv[1], "all"))
        {
            AswPlatM_PrintAllConfigInfo();
        }
        else if (0 == strcmp(argv[1], "errorLog"))
        {
            SSSnapshot_ExportItem(eSSSnapshotItemType_ErrorLog, eSnapshotItemReadSrc_Remote);
        }
        else if (0 == strcmp(argv[1], "runningLog"))
        {
            SSSnapshot_ExportItem(eSSSnapshotItemType_RunningLog, eSnapshotItemReadSrc_Remote);
        }
    }

    return 0;
}

static int32_t DSConsoleCfg_SetPara(int32_t argc, char *argv[])
{
    int32_t port = 0;
    uint8_t *pTemp = NULL;
    uint8_t setResult = FALSE;

    if (argc == 3)
    {
        /* 设置平台桩号 */
        if (0 == strcmp(argv[1], "dn"))
        {
            if (TRUE == AswPlatM_SetPileDn(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_LogPrint("Set PileDn: \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_LogPrint("Set PileDn failed!\r\n");
            }
        }
        /* 设置运维桩号 */
        else if (0 == strcmp(argv[1], "odn"))
        {
            if (TRUE == AswPlatM_SetFixPileDn(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_LogPrint("Set PileOdn: \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_LogPrint("Set PileOdn failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "plat"))
        {
            char platName[16 + 1] = {0};

            if (sscanf(argv[2], "%16s", platName) == 1)
            {
                if (TRUE == AswPlatM_SetPlatType(platName))
                {
                    setResult = TRUE;
                    DSCONSOLE_CFG_LogPrint("Set plat: \"%s\" ok!\r\n", platName);
                }
            }

            if (setResult == FALSE)
            {
                DSCONSOLE_CFG_LogPrint("Set plat failed!\r\n");
            } 
        }
        else if (0 == strcmp(argv[1], "card"))
        {
            char platCardName[16 + 1] = {0};

            if (sscanf(argv[2], "%16s", platCardName) == 1)
            {
                if (TRUE == AswPlatM_SetPlatCardType(platCardName))
                {
                    setResult = TRUE;
                    DSCONSOLE_CFG_LogPrint("Set card: \"%s\" ok!\r\n", platCardName);
                }
            }

            if (setResult == FALSE)
            {
                DSCONSOLE_CFG_LogPrint("Set card failed!\r\n");
            } 
        }
        /* 设置参数 */
        else if (0 == strcmp(argv[1], "para"))
        {
            /* 设置IP端口 */
            if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "ip:", strlen("ip:"))))
            {
                pTemp += strlen("ip:");

                if (strlen((char *)pTemp) < CDD_NETM_CFG_IP_LEN)
                {
                    char ip[CDD_NETM_CFG_IP_LEN + 1] = {0};

                    if (sscanf((char *)pTemp, "%72[^,],%d", ip, &port) == 2)
                    {
                        if (TRUE == AswPlatM_SetPlatMainIpPort(ip, strlen(ip), (uint16_t)port))
                        {
                            DSCONSOLE_CFG_LogPrint("Set Plat Main ip port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_LogPrint("Set Plat Main ip port failed!\r\n");
                }
            }
            else if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "domain:", strlen("domain:"))))
            {
                pTemp += strlen("domain:");

                if (strlen((char *)pTemp) < CDD_NETM_CFG_IP_LEN)
                {
                    char ip[CDD_NETM_CFG_IP_LEN + 1] = {0};

                    if (sscanf((char *)pTemp, "%72[^,],%d", ip, &port) == 2)
                    {
                        if (TRUE == AswPlatM_SetPlatMainIpPort(ip, strlen(ip), (uint16_t)port))
                        {
                            DSCONSOLE_CFG_LogPrint("Set Plat Main domain port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_LogPrint("Set Plat Main domain port failed!\r\n");
                }
            }
            else if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "port:", strlen("port:"))))
            {
                pTemp += strlen("port:");
                if (sscanf((char *)pTemp, "%d", &port) == 1)
                {
                    if (TRUE == AswPlatM_SetPlatMainPort((uint16_t)port))
                    {
                        DSCONSOLE_CFG_LogPrint("Set Plat Main port= %d ok!\r\n", port);
                        setResult = TRUE;
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_LogPrint("Set Plat Main port failed!\r\n");
                }
            }
            else
            {}
        }

        /* 设置YKC21密钥 */
        else if (0 == strcmp(argv[1], "ykc21key"))
        {
            if (TRUE == AswPlatM_Setykc21key(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_LogPrint("Set ykc21key \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_LogPrint("Set ykc21key failed! erroplat or len > 128\r\n");
            }
        }
        /* 设置YKC21token  */
        else if (0 == strcmp(argv[1], "ykc21token"))
        {
            if (TRUE == AswPlatM_Setykc21token(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_LogPrint("Set ykc21token \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_LogPrint("Set ykc21token failed! erroplat or len > 14\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "mntr"))
        {
            /* 设置IP端口 */
            if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "ip:", strlen("ip:"))))
            {
                pTemp += strlen("ip:");

                if (strlen((char *)pTemp) < CDD_NETM_CFG_IP_LEN)
                {
                    char ip[CDD_NETM_CFG_IP_LEN + 1] = {0};

                    if (sscanf((char *)pTemp, "%72[^,],%d", ip, &port) == 2)
                    {
                        if (TRUE == AswPlatM_SetPlatAuxiliaryIpPort(ip, strlen(ip), (uint16_t)port))
                        {
                            DSCONSOLE_CFG_LogPrint("Set Plat Auxiliary ip port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_LogPrint("Set Plat Auxiliary ip port failed!\r\n");
                }
            }
            else if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "port:", strlen("port:"))))
            {
                pTemp += strlen("port:");
                if (sscanf((char *)pTemp, "%d", &port) == 1)
                {
                    if (TRUE == AswPlatM_SetPlatAuxiliaryPort((uint16_t)port))
                    {
                        DSCONSOLE_CFG_LogPrint("Set Plat Auxiliary port= %d ok!\r\n", port);
                        setResult = TRUE;
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_LogPrint("Set Plat Auxiliary port failed!\r\n");
                }

            }
        }
    }

    return 0;
}

static int32_t DSConsoleCfg_ReadAgingState(int32_t argc, char *argv[])
{
    DSCONSOLE_CFG_LogPrint("AgingState: %d\r\n", CddModeM_IsAgingTestFinish());
    return 0;
}













