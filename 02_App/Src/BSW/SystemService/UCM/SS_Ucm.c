/******************************************************************************
* File Name          : template.c
* Description        : Code for xxxxxxxxxxx
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
#include "SS_Ucm.h"
#include "SS_UcmConfig.h"
#include "fal_cfg.h"
#include "fal.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
const char* c_UcmResultStr[] =
{
    "None",
    "下载文件成功",
    "获取文件失败",
    "文件头错误",
    "数据接收中断",
    "升级超时",
    "模组空间不足",
    "未知错误"
};

/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eSSUcmBootState_Idle,
    eSSUcmBootState_FileCheck,
    eSSUcmBootState_Backup,    
    eSSUcmBootState_Copy,
    eSSUcmBootState_RollBack,
    eSSUcmBootState_Jump,
}SSUcmBootState_Enum;


/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    SSUcmBootState_Enum eBootState;
    uint32_t bootVer;
    uint8_t res[123];
}SSUcmPara_Struct;

typedef struct
{
    uint8_t     flagUp1[4];           /* 升级论证标志1 0x12345678 */    
    char        rtuType[16];          /* 终端类型 */ 
    uint8_t     softVer[4];           /* 软件版本号 */ 
    uint8_t     softDate[4];          /* 软件日期 */ 
    uint8_t     binFileLen[4];        /* bin文件大小 程序大小不包括打包头 */ 
    uint8_t     flagUp2[4];           /* 升级论证标志2 0x87654321 */ 
    uint8_t     cs32[4];              /* 程序文件累加和校验 */ 
    uint8_t     crc_16[2];            /* 从flagUp1到flagUp2的crc16校验 */ 
    uint8_t     rvsd[86];             /* 预留 */ 
}SSUcmFileHead_Struct;

typedef struct 
{
    SSUcmWorkState_Enum eUcmWorkState;     /* 更新状态 */
    
    CddNetMSocketPara_Union strNetPara;
    eSSUcmChannelType_Enum eChannelType;
    uint32_t    timeoutThresold;
    uint32_t    timeoutTick;
    uint8_t     delayFlag;                 /* 延时执行标记 */

	uint8_t	    packCnt;                   /* 包数量 */
	uint8_t	    packIndex;                 /* 包序号 */
	uint8_t	    packSize;                  /* 包大小 */

	uint32_t	totalFrameCnt;             /* 帧数量 */
	uint32_t	currentFrameIndex;         /* 帧序号 */
	uint32_t	frameLen;                  /* 帧长度 */
	uint32_t	finalFrameLen;             /* 最后一帧长度 */

    uint32_t    readLen;
    uint32_t    readOffset;

    uint32_t    fileSize;                  /* 总文件大小 */
    uint32_t    fileCs;                    /* 文件和校验 */
    SSUcmResult_Enum eResult;
}SSUcmCtx_Struct; 





/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static SSUcmCtx_Struct g_stSSUcmCtx =  { 0 };
static SSUcmPara_Struct g_stSSUcmPara = { 0 };

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static uint16_t SSUcm_Crc16(uint8_t* pInBuf, uint32_t nLen);
static uint8_t SSUcm_ErasePart(char *partName);
static uint8_t SSUcm_program_part(const char *part_name, uint32_t addr, const uint8_t *buf, size_t size);
static uint8_t SSUcm_read_part(const char *part_name, uint32_t addr, uint8_t *buf, size_t size);
static void SSUcm_DefaultUcmPara(void);
static void SSUcm_WriteUcmPara(void);
static void SSUcm_LoadUcmPara(void);
static uint8_t SSUcm_CheckFileHead(uint8_t *headData, uint32_t dataLen);
static void SSUcm_SetWorkState(SSUcmWorkState_Enum eWorkState);
/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static uint8_t SSUcm_ErasePart(char *partName)
{
    const struct fal_partition *part;
    part = fal_partition_find(partName);
    uint8_t ret = FALSE;

    if (part == NULL)
    {
        SSUCM_CFG_LogPrint("Partition\"%s\" not found.", partName);
    }
    else
    {
        if (fal_partition_erase(part, 0, part->len) >= 0)
        {
            ret = TRUE;
        }
        else
        {
            SSUCM_CFG_LogPrint("Partition\"%s\" erase failed!", part->name);
        }
    }

    return ret;
}

static uint8_t SSUcm_program_part(const char *part_name, uint32_t addr, const uint8_t *buf, size_t size)
{
    const struct fal_partition *part;
    part = fal_partition_find(part_name);
    uint8_t ret = FALSE;

    if (part == NULL)
    {
        SSUCM_CFG_LogPrint("Partition\"%s\" not found.", part_name);
    }
    else
    {
        if (fal_partition_write(part, addr, buf, size) >= 0)
        {
            ret = TRUE;
        }
        else
        {
            SSUCM_CFG_LogPrint("Partition\"%s\" write failed!", part->name);
        }
    }

    return ret;
}

static uint8_t SSUcm_read_part(const char *part_name, uint32_t addr, uint8_t *buf, size_t size)
{
    const struct fal_partition *part;
    part = fal_partition_find(part_name);
    uint8_t ret = FALSE;

    if (part == NULL)
    {
        SSUCM_CFG_LogPrint("Partition\"%s\" not found.", part_name);
    }
    else
    {
        if (fal_partition_read(part, addr, buf, size) >= 0)
        {
            ret = TRUE;
        }
        else
        {
            SSUCM_CFG_LogPrint("Partition\"%s\" read failed!", part->name);
        }
    }
    
    return ret;
}
static uint16_t SSUcm_Crc16(uint8_t* pInBuf, uint32_t nLen)
{
    uint32_t i, j;
    uint16_t wTemp = 0, wFlag = 0;
	
    for (i = 0; i < nLen; i++)
    {
        wTemp ^= *(pInBuf + i) << 8;
        for (j = 0; j < 8; j++)
        {
            wFlag = wTemp & 0x8000;
            wTemp <<= 1;
            if (wFlag)
            {
                wTemp ^= 0x1021;
            }
        }
    }
	
    return wTemp;
}

static void SSUcm_DefaultUcmPara(void)
{
    memset(&g_stSSUcmPara, 0x00, sizeof(SSUcmPara_Struct));
}

static void SSUcm_WriteUcmPara(void)
{
    if (SSUcm_ErasePart(FAL_NULL_NAME_UCM_PARA) == TRUE)
    {
        SSUcm_program_part(FAL_NULL_NAME_UCM_PARA, 0, (uint8_t *)&g_stSSUcmPara, sizeof(SSUcmPara_Struct));
    }
}

static void SSUcm_LoadUcmPara(void)
{
    if (TRUE != SSUcm_read_part(FAL_NULL_NAME_UCM_PARA, 0, (uint8_t *)&g_stSSUcmPara, sizeof(SSUcmPara_Struct)))
    {
        SSUCM_CFG_LogPrint("Ucm 参数加载失败, 执行默认参数!\r\n");
        SSUcm_DefaultUcmPara();
    }
}

static uint8_t SSUcm_CheckFileHead(uint8_t *headData, uint32_t dataLen)
{
    SSUcmFileHead_Struct *pFileHead = (SSUcmFileHead_Struct *)headData;
    uint8_t ret = FALSE;
    uint16_t calcCrc = 0, recvCrc = 0;
    uint32_t fileLen = 0;

    calcCrc = SSUcm_Crc16(pFileHead->flagUp1, STRUCT_POS(SSUcmFileHead_Struct, crc_16));
    recvCrc = Common_TwoUint8ToUint16(pFileHead->crc_16);

    if (calcCrc == recvCrc)
    {
        fileLen = Common_FourUint8ToUint32(pFileHead->binFileLen);

        if (fileLen >= SSUCM_CONFIG_SINGLE_FRAME_LEN)
        {
            ret = TRUE;
        }
    }
    else
    {
        SSUCM_CFG_LogPrint("文件头校验CRC错误, recvCrc: 0x%04X, calcCrc: 0x%04X!\r\n", recvCrc, calcCrc);
    }

    return ret;
}

static uint8_t SSUcm_FIleHeadHandle(uint8_t *headData, uint32_t dataLen)
{
    SSUcmFileHead_Struct *pFileHead = (SSUcmFileHead_Struct *)headData;
    uint8_t ret = FALSE;

    if (pFileHead != NULL && dataLen > sizeof(SSUcmFileHead_Struct))
    {
        if (SSUcm_CheckFileHead(headData, dataLen) == TRUE)
        {
            g_stSSUcmCtx.fileSize = Common_FourUint8ToUint32(pFileHead->binFileLen) + sizeof(SSUcmFileHead_Struct);
            g_stSSUcmCtx.fileCs = Common_TwoUint8ToUint16(pFileHead->crc_16);

            g_stSSUcmCtx.packIndex = 0;
            g_stSSUcmCtx.packCnt = (g_stSSUcmCtx.fileSize + SSUCM_CONFIG_CUT_PACK_SIZE - 1) / SSUCM_CONFIG_CUT_PACK_SIZE;

            g_stSSUcmCtx.totalFrameCnt = (g_stSSUcmCtx.fileSize + SSUCM_CONFIG_SINGLE_FRAME_LEN - 1) / SSUCM_CONFIG_SINGLE_FRAME_LEN;

            g_stSSUcmCtx.finalFrameLen = g_stSSUcmCtx.fileSize % SSUCM_CONFIG_SINGLE_FRAME_LEN;
            g_stSSUcmCtx.finalFrameLen = (g_stSSUcmCtx.finalFrameLen == 0) ? SSUCM_CONFIG_SINGLE_FRAME_LEN : g_stSSUcmCtx.finalFrameLen;
            ret = TRUE;
        }
    }

    return ret;
}

static void SSUcm_SetWorkState(SSUcmWorkState_Enum eWorkState)
{
    SSUcmWorkState_Enum ePreWorkState;

    if (g_stSSUcmCtx.eUcmWorkState != eWorkState)
    {
        ePreWorkState = g_stSSUcmCtx.eUcmWorkState;
        g_stSSUcmCtx.eUcmWorkState = eWorkState;

        if (eWorkState == eSSUcmWorkState_Connecting)
        {
            g_stSSUcmCtx.timeoutTick = Common_GetSystick();
        }
        else if (eWorkState == eSSUcmWorkState_Finish)
        {
            if (ePreWorkState >= eSSUcmWorkState_Connecting && ePreWorkState < eSSUcmWorkState_Finish)
            {
                CddNetM_DeleteLink(eCddNetMPlatType_File);
            }

            if (g_stSSUcmCtx.eResult == eSSUcmResult_Succ)
            {
                g_stSSUcmCtx.eUcmWorkState = eSSUcmWorkState_Idle;
                g_stSSUcmPara.eBootState = eSSUcmBootState_FileCheck;
                SSUcm_WriteUcmPara();
                SSUCM_CFG_Reboot();
                SSUCM_CFG_LogPrint("升级文件下载完成，即将重启...!\r\n");
            }
            else
            {
                SSUCM_CFG_LogPrint("升级失败, 失败原因：%s!\r\n", c_UcmResultStr[g_stSSUcmCtx.eResult]);
                g_stSSUcmCtx.eUcmWorkState = eSSUcmWorkState_Idle;
            }
        }
    }
}

static void SSUcm_TimeoutHandle(void)
{
    if (g_stSSUcmCtx.eUcmWorkState >= eSSUcmWorkState_Connecting && 
        g_stSSUcmCtx.eUcmWorkState < eSSUcmWorkState_Finish)
    {
        if (Common_JudgeTimeoutMs(g_stSSUcmCtx.timeoutTick, g_stSSUcmCtx.timeoutThresold) == TRUE)
        {
            SSUcm_SetResult(eSSUcmResult_Timeout);
        }
    }
}

static void SSUcm_RollbackCheck(void)
{
    if (g_stSSUcmPara.eBootState == eSSUcmBootState_RollBack)
    {
        if (Common_JudgeTimeoutMs(g_stSSUcmCtx.timeoutTick, SSUCM_CONFIG_STABLE_TIMEOUT))
        {
            SSUCM_CFG_LogPrint("升级后, 稳定15秒成功!\r\n");
            g_stSSUcmPara.eBootState = eSSUcmBootState_Idle;
            SSUcm_WriteUcmPara();
        }
    }
}

uint8_t SSUcm_FileDataHandle(uint8_t *data, uint32_t dataLen)
{
    uint8_t headOKflag = TRUE;
    uint32_t writeAddr = 0;
    uint8_t ret = FALSE;

    if (g_stSSUcmCtx.currentFrameIndex == 0)
    {
        if (SSUcm_FIleHeadHandle(data, dataLen) != TRUE)
        {
            SSUcm_SetResult(eSSUcmResult_HeadErr);
            headOKflag = FALSE;
        }
        else
        {
            SSUcm_ErasePart(FAL_NULL_NAME_UPDATE_PROGRAM);
        }
    }

    if (headOKflag == TRUE)
    {
        if (dataLen == g_stSSUcmCtx.readLen)
        {
            SSUCM_CFG_LogPrint("文件下载中[进度：%d%%]: 总包数[%d], 当前包数[%d], 总帧数[%d], 当前帧数[%d]\r\n",
                g_stSSUcmCtx.currentFrameIndex * 100 / g_stSSUcmCtx.totalFrameCnt,
                 g_stSSUcmCtx.packCnt, g_stSSUcmCtx.packIndex + 1,
                g_stSSUcmCtx.totalFrameCnt,g_stSSUcmCtx.currentFrameIndex);

            writeAddr = g_stSSUcmCtx.currentFrameIndex * SSUCM_CONFIG_SINGLE_FRAME_LEN;
            SSUcm_program_part(FAL_NULL_NAME_UPDATE_PROGRAM, writeAddr,  data, dataLen);

            g_stSSUcmCtx.currentFrameIndex++;

            if (g_stSSUcmCtx.currentFrameIndex >= g_stSSUcmCtx.totalFrameCnt)
            {
                SSUcm_SetResult(eSSUcmResult_Succ);
            }
            else 
            {
                if (g_stSSUcmCtx.currentFrameIndex != 0 && (g_stSSUcmCtx.currentFrameIndex % SSUCM_CONFIG_PACK_FRAME_CNT) == 0)
                {
                    g_stSSUcmCtx.packIndex++;
                    ret = TRUE;
                }

                g_stSSUcmCtx.readOffset = (g_stSSUcmCtx.currentFrameIndex % SSUCM_CONFIG_PACK_FRAME_CNT) * SSUCM_CONFIG_SINGLE_FRAME_LEN;

                if (g_stSSUcmCtx.currentFrameIndex + 1 == g_stSSUcmCtx.totalFrameCnt)
                {
                    g_stSSUcmCtx.readLen = g_stSSUcmCtx.finalFrameLen;
                }
                else
                {
                    g_stSSUcmCtx.readLen = SSUCM_CONFIG_SINGLE_FRAME_LEN;
                }

                g_stSSUcmCtx.readLen = (g_stSSUcmCtx.readLen > SSUCM_CONFIG_SINGLE_FRAME_LEN) ? SSUCM_CONFIG_SINGLE_FRAME_LEN : g_stSSUcmCtx.readLen;
            }
        }
        else
        {
            SSUCM_CFG_LogPrint("Download file error, recvLen: %d, readLen: %d\r\n", dataLen, g_stSSUcmCtx.readLen);
            SSUcm_SetResult(eSSUcmResult_DataRecvInterrupt);
        }
    }

    return ret;
}

uint8_t SSUcm_CheckUpdateCondition(void)
{
    uint8_t ret = TRUE;
    uint8_t port = 0;
    
    for (port = 0; port < SYSCFG_CFG_GUN_NUM; port++)
    {
        if (AswCharge_GetWorkState(port) != ASWCHARGE_WORKSTATE_IDLE)
        {
            ret = FALSE;
            break;
        }
    }
    
    if (ret == TRUE && CddNetM_CheckFileLinkExsit() == TRUE)
    {
        ret = FALSE;
    }

    return ret;
}

SSUcmWorkState_Enum SSUcm_GetWorkState(void)
{
    return g_stSSUcmCtx.eUcmWorkState;
}

uint8_t SSUcm_GetReadLenAndOffSet(uint16_t *pReadLen, uint32_t* pReadOffset)
{
    uint8_t ret = FALSE;

    if (pReadLen != NULL && pReadOffset != NULL)
    {
        *pReadLen = g_stSSUcmCtx.readLen;
        *pReadOffset = g_stSSUcmCtx.readOffset;
        ret = TRUE;
    }

    return ret;
}

uint8_t SSUcm_GetPackIndex(uint8_t *pPackIndex)
{
    uint8_t ret = FALSE;

    if (pPackIndex != NULL)
    {
        *pPackIndex = g_stSSUcmCtx.packIndex;
        ret = TRUE;
    }

    return ret;
}

uint8_t SSUcm_IsUpdating(void)
{
    uint8_t ret = FALSE;

    if (g_stSSUcmCtx.eUcmWorkState >= eSSUcmWorkState_Connecting)
    {
        ret = TRUE;
    }

    return ret;
}

uint8_t SSUcm_IsOngoging(void)
{
    uint8_t ret = FALSE;

    if (g_stSSUcmCtx.eUcmWorkState >= eSSUcmWorkState_WaitIdle)
    {
        ret = TRUE;
    }

    return ret;
}


void SSUcm_SetResult(SSUcmResult_Enum eResult)
{
    if (g_stSSUcmCtx.eUcmWorkState != eSSUcmWorkState_Idle)
    {
        if (g_stSSUcmCtx.eResult == eSSUcmResult_None && eResult != eSSUcmResult_None)
        {
            g_stSSUcmCtx.eResult = eResult;
            SSUcm_SetWorkState(eSSUcmWorkState_Finish);
        }
    }
}

SSUcmResult_Enum SSUcm_GetResult(void)
{
    SSUcmResult_Enum eResult = g_stSSUcmCtx.eResult;

    g_stSSUcmCtx.eResult = eSSUcmResult_None;
    return eResult;
}

void SSUcm_ReqStartOTA(CddNetMSocketPara_Union *pNetPara, eSSUcmChannelType_Enum eChannelType, 
    eSSUcmExcuteMode_Enum eExcuteMode, uint32_t timeout)
{
    if (g_stSSUcmCtx.eUcmWorkState == eSSUcmWorkState_Idle)
    {
        memset(&g_stSSUcmCtx, 0x00, sizeof(g_stSSUcmCtx));

        if (pNetPara != NULL && eChannelType < eSSUcmChannelType_Count)
        {
            memcpy(&g_stSSUcmCtx.strNetPara, pNetPara, sizeof(CddNetMSocketPara_Union));
            g_stSSUcmCtx.eChannelType = eChannelType;
            g_stSSUcmCtx.readLen = SSUCM_CONFIG_SINGLE_FRAME_LEN;
            g_stSSUcmCtx.delayFlag = TRUE;

            if (timeout == 0)
            {
                g_stSSUcmCtx.timeoutThresold = SSUCM_CONFIG_TIMEOUT_MS;
            }
            else
            {
                g_stSSUcmCtx.timeoutThresold = timeout;
            }

            if (eExcuteMode == eSSUcmExcuteMode_Immediate)
            {
                SSUcm_SetWorkState(eSSUcmWorkState_Connecting);
                SSUCM_CFG_LogPrint("请求升级成功,立即执行!\r\n");
            }
            else
            {
                SSUcm_SetWorkState(eSSUcmWorkState_WaitIdle);
                SSUCM_CFG_LogPrint("请求升级成功,等待空闲执行!\r\n");
            }
        }
    }
}

void SSUcm_InitMemory(void)
{ 
    memset(&g_stSSUcmCtx, 0x00, sizeof(SSUcmCtx_Struct));
    SSUcm_LoadUcmPara();
}

void SSUcm_MainFunction(void)
{
    if (g_stSSUcmCtx.eUcmWorkState == eSSUcmWorkState_WaitIdle)
    {
        if (SSUcm_CheckUpdateCondition() == TRUE)
        {
            SSUcm_SetWorkState(eSSUcmWorkState_Connecting);
        }
    }
    else if (g_stSSUcmCtx.eUcmWorkState == eSSUcmWorkState_Connecting)
    {
        if (g_stSSUcmCtx.delayFlag == TRUE)
        {
            if (Common_JudgeTimeoutMs(g_stSSUcmCtx.timeoutTick, 2000) == TRUE)
            {
                g_stSSUcmCtx.delayFlag = FALSE;

                if (eGlobalRet_OK != CddNetM_CreatLink(eCddNetMSocketType_FTP, g_stSSUcmCtx.strNetPara, eCddNetMPlatType_File))
                {
                    SSUcm_SetResult(eSSUcmResult_UnexpectedError);
                }
            }
        }
    }

    SSUcm_TimeoutHandle();

    SSUcm_RollbackCheck();
}


























