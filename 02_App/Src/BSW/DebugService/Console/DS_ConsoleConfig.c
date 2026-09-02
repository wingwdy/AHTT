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


#include "Asw_PlatM.h"
#include "Asw_Monitor.h"
#include "Asw_ErrorHandle.h"

#include "Cdd_ModeM.h"
#include "Cdd_NetM.h"

#include "SS_Snapshot.h"
#include "SS_Ucm.h"

#include "fal.h"
#include "fal_cfg.h"

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
static int32_t DSConsoleCfg_ShowStack(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_EnterFactoryMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ExsistFactoryMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_HandleGbMode(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_SetPara(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_GetPara(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ResetPara(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ReadAgingState(int32_t argc, char *argv[]);
static int32_t DSConsoleCfg_ClearFlash(int32_t argc, char *argv[]);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
DSCONSOLE_CFG_ADD_CMD(reboot,        DSConsoleCfg_Reboot, "reboot" reboot system);
DSCONSOLE_CFG_ADD_CMD(showStack,     DSConsoleCfg_ShowStack, "showStack" show stack info);
DSCONSOLE_CFG_ADD_CMD(testmode,      DSConsoleCfg_EnterFactoryMode, "testmode" Enter factoryMode);
DSCONSOLE_CFG_ADD_CMD(workmode,      DSConsoleCfg_ExsistFactoryMode, "workmode" Exsist factoryMode);
DSCONSOLE_CFG_ADD_CMD(gbmode,        DSConsoleCfg_HandleGbMode, "gbmode 1 / 2" EnterGbMode/ExsitGbMode);
DSCONSOLE_CFG_ADD_CMD(set,           DSConsoleCfg_SetPara, "set xxx" set param);
DSCONSOLE_CFG_ADD_CMD(get,           DSConsoleCfg_GetPara, "get xxx" get param);
DSCONSOLE_CFG_ADD_CMD(ReadAgingState,DSConsoleCfg_ReadAgingState, "ReadAgingState" ReadAgingState);
DSCONSOLE_CFG_ADD_CMD(clearFlash,    DSConsoleCfg_ClearFlash,     "clearFlash"    clearFlash);
DSCONSOLE_CFG_ADD_CMD(reset,         DSConsoleCfg_ResetPara, "reset xxx" reset param);

static int32_t DSConsoleCfg_Reboot(int32_t argc, char *argv[])
{
    AswMonitor_SetReboot(eAswMonitorRebootType_Immediate);
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
            SSSnapshot_FlushRunningLog();
            SSSnapshot_ExportItem(eSSSnapshotItemType_RunningLog, eSnapshotItemReadSrc_Remote);
        }
        else if (0 == strcmp(argv[1], "orderRecord"))
        {
            SSSnapshot_ExportItem(eSSSnapshotItemType_OmOrderRecord, eSnapshotItemReadSrc_Remote);
        }
        else if (0 == strcmp(argv[1], "allLog"))
        {
            SSSnapshot_ExportAllItems(NULL);
        }
    }

    return 0;
}

static int32_t DSConsoleCfg_SetPara(int32_t argc, char *argv[])
{
    int32_t port = 0;
    uint8_t *pTemp = NULL;
    uint8_t setResult = FALSE;
    uint32_t temp = 0;

    if (argc == 3)
    {
        /* 设置平台桩号 */
        if (0 == strcmp(argv[1], "dn"))
        {
            if (TRUE == AswPlatM_SetPileDn(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set PileDn: \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set PileDn failed!\r\n");
            }
        }
        /* 设置运维桩号 */
        else if (0 == strcmp(argv[1], "odn"))
        {
            if (TRUE == AswPlatM_SetFixPileDn(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set PileOdn: \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set PileOdn failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "plat"))
        {
            char platName[16 + 1] = {0};

            if (sscanf(argv[2], "%16s", platName) == 1)
            {
                if (TRUE == AswPlatM_SetPlatType(platName, strlen(platName)))
                {
                    setResult = TRUE;
                    DSCONSOLE_CFG_InfoPrint("Set plat: \"%s\" ok!\r\n", platName);
                }
            }

            if (setResult == FALSE)
            {
                DSCONSOLE_CFG_InfoPrint("Set plat failed!\r\n");
            } 
        }
        else if (0 == strcmp(argv[1], "card"))
        {
            char platCardName[16 + 1] = {0};

            if (sscanf(argv[2], "%16s", platCardName) == 1)
            {
                if (TRUE == AswPlatM_SetPlatCardType(platCardName, strlen(platCardName)))
                {
                    setResult = TRUE;
                    DSCONSOLE_CFG_InfoPrint("Set card: \"%s\" ok!\r\n", platCardName);
                }
            }

            if (setResult == FALSE)
            {
                DSCONSOLE_CFG_InfoPrint("Set card failed!\r\n");
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
                            DSCONSOLE_CFG_InfoPrint("Set Plat Main ip port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_InfoPrint("Set Plat Main ip port failed!\r\n");
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
                            DSCONSOLE_CFG_InfoPrint("Set Plat Main domain port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_InfoPrint("Set Plat Main domain port failed!\r\n");
                }
            }
            else if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "port:", strlen("port:"))))
            {
                pTemp += strlen("port:");
                if (sscanf((char *)pTemp, "%d", &port) == 1)
                {
                    if (TRUE == AswPlatM_SetPlatMainPort((uint16_t)port))
                    {
                        DSCONSOLE_CFG_InfoPrint("Set Plat Main port= %d ok!\r\n", port);
                        setResult = TRUE;
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_InfoPrint("Set Plat Main port failed!\r\n");
                }
            }
            else
            {}
        }
        else if (0 == strcmp(argv[1], "cipherKey"))
        {
            if (TRUE == AswPlatM_SetCipherKey(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set cipherKey \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set cipherKey failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "token"))
        {
            if (TRUE == AswPlatM_SetToken(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set token \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set token failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "devOperator"))
        {
            if (TRUE == AswPlatM_SetDevOperator(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set devOperator \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set devOperator failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "productKey"))
        {
            if (TRUE == AswPlatM_SetProductKey(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set productKey \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set productKey failed!\r\n");
            }
        }
        else if (0 == strcmp(argv[1], "productSecret"))
        {
            if (TRUE == AswPlatM_SetProductSecret(argv[2], strlen(argv[2])))
            {
                DSCONSOLE_CFG_InfoPrint("Set productSecret \"%s\" ok!\r\n", argv[2]);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set productSecret failed!\r\n");
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
                            DSCONSOLE_CFG_InfoPrint("Set Plat Auxiliary ip port: \"%s\", port= %d ok!\r\n", ip, port);
                            setResult = TRUE;
                        }
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_InfoPrint("Set Plat Auxiliary ip port failed!\r\n");
                }
            }
            else if (NULL != (pTemp = Common_SearchData((uint8_t *)argv[2], strlen(argv[2]), "port:", strlen("port:"))))
            {
                pTemp += strlen("port:");
                if (sscanf((char *)pTemp, "%d", &port) == 1)
                {
                    if (TRUE == AswPlatM_SetPlatAuxiliaryPort((uint16_t)port))
                    {
                        DSCONSOLE_CFG_InfoPrint("Set Plat Auxiliary port= %d ok!\r\n", port);
                        setResult = TRUE;
                    }
                }

                if (setResult == FALSE)
                {
                    DSCONSOLE_CFG_InfoPrint("Set Plat Auxiliary port failed!\r\n");
                }

            }
        }
        else if (0 == strcmp(argv[1], "omPlatDisable"))
        {
            temp = atoi(argv[2]);

            if (TRUE == AswPlatM_SetOmPlatDisable(temp))
            {
                DSCONSOLE_CFG_InfoPrint("Set omPlat Disable= %d ok!\r\n", temp);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set omPlat Disable= %d failed!\r\n", temp);
            }
        }
        else if (0 == strcmp(argv[1], "simNet"))
        {
            temp = atoi(argv[2]);

            if (TRUE == AswPlatM_SetSimNet(temp))
            {
                DSCONSOLE_CFG_InfoPrint("Set simNet= %d ok!, %s\r\n", temp, temp == 0 ? "公网卡" : "专网卡");
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set simNet= %d failed!\r\n", temp);
            }
        }
        /* 用于测试故障 */
        else if (0 == strcmp(argv[1], "errState"))
        {
            temp = atoi(argv[2]);
            AswErrHandle_SetErrStateForTest(0, temp);
        }
        /* 设置环境标志 */
        else if (0 == strcmp(argv[1], "env"))
        {
            temp = atoi(argv[2]);

            if (TRUE == AswPlatM_SetEnvFlag(temp))
            {
                DSCONSOLE_CFG_InfoPrint("Set env= %d ok!\r\n", temp);
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Set env= %d failed!\r\n", temp);
            }
        }
        else
        {
            DSCONSOLE_CFG_InfoPrint("Invalid command!\r\n");
        }
    }

    return 0;
}

static int32_t DSConsoleCfg_ReadAgingState(int32_t argc, char *argv[])
{
    DSCONSOLE_CFG_InfoPrint("AgingState: %d\r\n", CddModeM_IsAgingTestFinish());
    return 0;
}

static int32_t DSConsoleCfg_ClearFlash(int32_t argc, char *argv[])
{
    int32_t ret = -1;
    DSCONSOLE_CFG_DebugPrint("开始格式化Flash....\r\n");

    const struct fal_flash_dev *flash_dev = fal_flash_device_find(NOR_FLASH_DEV_NAME);
    if (flash_dev != NULL)
    {
        ret = flash_dev->ops.erase(0, flash_dev->len);
        if (ret >= 0)
        {
            DSCONSOLE_CFG_DebugPrint("格式化Flash完成！\r\n");
        }
        else
        {
            DSCONSOLE_CFG_DebugPrint("格式化Flash失败！错误码: %d\r\n", ret);
        }
    }
    else
    {
        DSCONSOLE_CFG_DebugPrint("获取Flash设备失败！\r\n");
    }

    return ret;
}

static int32_t DSConsoleCfg_ResetPara(int32_t argc, char *argv[])
{
    GlobalRet_Enum eRet = eGlobalRet_ParaInvalid;

    if (argc == 2)
    {
        if (0 == strcmp(argv[1], "platPrivatePara"))
        {
            eRet = MSNvm_SetDefaultParaBlock(eMSNvmBlockID_PlatPrivateParam);

            if (eRet == eGlobalRet_OK)
            {
                DSCONSOLE_CFG_InfoPrint("Reset platPrivatePara ok!\r\n");
            }
            else
            {
                DSCONSOLE_CFG_InfoPrint("Reset platPrivatePara failed, result: %d!\r\n", eRet);
            }
        }
    }

    return 0;
}











