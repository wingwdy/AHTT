/******************************************************************************
* File Name          : Cdd_CardM.c
* Description        : Code for Card Manage
 -------------------------------------------------------------------------------
* (c) This software is the proprietary of Bull. All rights are reserved by Bull.
-------------------------------------------------------------------------------
*             R E V I S I O N   H I S T O R Y
-------------------------------------------------------------------------------
* Date          Version      Author    Description
------------    --------     -------   ----------------------------------------
*2025/10/10      V1.0.0      sjc    初版创建
*
*******************************************************************************/


/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/
#include "Cdd_CardM.h"
#include "Cdd_CardMConfig.h"
#include "Cdd_Drv_LS5120.h"
#include "SysCfg.h"
#include "md5.h"
#include "Asw_ErrorHandle.h"

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define BULLCARD_USERCARD_SECRET                "bullevse666_user" /* 公牛用户卡秘钥 */
#define BULLCARD_CARDID_ADDR                    (4u) /* 公牛卡号绝对地址 */
#define BULLCARD_CARDID_LEN                     (8U) 

#define XIAOJU_CARD_CARDID_LEN                  (16U)   /* 小桔平台原始卡号字节数 */
#define XIAOJU_CARD_CRCSERIAL_ADDR              (4U)    /* 序列数存放地址 扇区1块0 */
#define XIAOJU_CARD_CRCVALUE_ADDR               (5U)    /* CRC值存放地址 扇区1块1 */
#define XIAOJU_CARD_CARDID_ADDR                 (8U)    /* 卡号存放地址 扇区2块0 */
#define XIAOJU_CARD_RANDOM1_ADDR                (20U)   /* 随机数1存放地址 扇区5块0 */
#define XIAOJU_CARD_RANDOM2_ADDR                (21U)   /* 随机数2存放地址 扇区5块1 */
#define XIAOJU_CARD_RANDOM3_ADDR                (22U)   /* 随机数3存放地址 扇区5块2 */
const uint8_t c_XiaojuCardSector1KEYA[6] = {0x34,0x71,0x4C,0x80,0x01,0x77}; /* 扇区1秘钥A */
const uint8_t c_XiaojuCardSector2KEYA[6] = {0x52,0x11,0x1A,0xB3,0x93,0x55}; /* 扇区2秘钥A */
const uint8_t c_XiaojuCardSector5KEYA[6] = {0x92,0x5C,0x9A,0x4B,0x83,0x74}; /* 扇区5秘钥A */

const char* c_CardTypeNameStr[] =
{
    "通用卡",
    "公牛卡",
    "小桔卡",
    "未知卡"
};

/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eNfcInit = 0,
    eNfcReady,
    eNfcPause,
	eNfcFault,
}eNfcState_Enum;

typedef enum
{
    eOptStepIdle = 0,
    eOptStepReadUID,
    eOptStepReadUserId,
}eNfcOptStep_Enum;

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/

typedef struct
{
    uint8_t cardUid[4]; /* 物理卡号 */
    uint8_t cardId[16]; /* 卡号 */
    uint8_t random1[16]; /* 随机数1 */
    uint8_t random2[16]; /* 随机数2 */
    uint8_t random3[16]; /* 随机数3 */
    uint8_t crcSerial[16]; /* CRC用序列数*/
    uint8_t crcCheck[16];
}CardInfo_Struct;

typedef struct 
{
    uint8_t initStep;
    eNfcOptStep_Enum nfcOptStep;
	eNfcState_Enum eNfcState;
    uint32_t nfcTick;
    uint32_t swipeCardInterval;
	uint8_t nfcFaultState;
    uint16_t nfcHardFaultCnt;
    CddCardType_Enum eCardType;
	CddCardType_Enum eCardTypeSet;
    CddCardEvent_Enum eCardEvent;
	CddCardEvent_Enum eCardEventOut;
    CardInfo_Struct stCardInfo;
}CddCardM_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/
static CddCardM_Struct g_stCddCardM;

/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/

static uint8_t CddCardM_GetBullCardSecretKeyA(const uint8_t *pUid, const uint8_t *pBullCardSecret, uint8_t *pkeyAOut);
static uint8_t CddCardM_ReadSector(uint8_t addr, const uint8_t *pUid, const uint8_t *pKeyA, uint8_t *pSectorData);
static uint8_t CddCardM_ReadCardUid(uint8_t *pTag, uint8_t *pCardUidOut, uint8_t *pResultOut);
static uint8_t CddCardM_BullCardIdCheck(const uint8_t *pUid, uint8_t *pSectorData);
static uint8_t CddCardM_ReadBullCardUserId(uint8_t addr, const uint8_t *pUid, uint8_t *pCardUserIdOut);
static uint8_t CddCardM_ReadXiaojuCardInfo(CardInfo_Struct *pInfo);
static void CddCardM_NfcInitProcess(CddCardM_Struct *pCardM);
static void CddCardM_NfcReadyProcess(CddCardM_Struct *pCardM);
static void CddCardM_NfcPauseProcess(CddCardM_Struct *pCardM);
static void CddCardM_NfcFaultProcess(CddCardM_Struct *pCardM);

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

/* 获取公牛卡块KEYA*/
static uint8_t CddCardM_GetBullCardSecretKeyA(const uint8_t *pUid, const uint8_t *pBullCardSecret, uint8_t *pkeyAOut)
{
    uint8_t ret = TRUE;
    uint8_t i = 0;
    uint8_t encrypt_ascii[28] = {0};
    uint8_t xor_result[8] = {0};
    uint8_t md5_decrypt[16] = {0};

    if(NULL == pUid || NULL == pBullCardSecret || NULL == pkeyAOut)
    {
        ret = FALSE;
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            Common_CvtHex2Ascii(pUid[i], encrypt_ascii + 2*i);
        }

        memcpy(encrypt_ascii + 8, pBullCardSecret, 20);

        MD5_API(md5_decrypt, encrypt_ascii, 24);

        for (i = 0; i < 8; i++)
        {
            xor_result[i] = md5_decrypt[i] ^ md5_decrypt[15 - i];
        }

        pkeyAOut[0] = xor_result[0] ^ xor_result[7];
        pkeyAOut[1] = xor_result[1] ^ xor_result[6];
        memcpy(pkeyAOut + 2, xor_result + 2, 4);
    }

    return ret;
}

static uint8_t CddCardM_ReadSector(uint8_t addr, const uint8_t *pUid, const uint8_t *pKeyA, uint8_t *pSectorData)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;

    if (pUid == NULL || pKeyA == NULL || pSectorData == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        if (optStep == 0)
        {
            optStatus = CddDrvLS5120_PcdAuthKeyA(addr, pKeyA, pUid);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
                if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    optStep = 1;
                }
            }
        }

        if (optStep == 1)
        {
            optStatus = CddDrvLS5120_PcdReadSector(addr, pSectorData);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
            }
        }
    }

    return optStatus;
}

static uint8_t CddCardM_ReadCardUid(uint8_t *pTag, uint8_t *pCardUidOut, uint8_t *pResultOut)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    static uint8_t Uid[4] = {0};
	uint8_t uIdLen = 0;

    if (pTag == NULL || pCardUidOut == NULL || pResultOut == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        *pResultOut = eGlobalRet_Error;

        if (optStep == 0)
        {
            memset(Uid, 0, sizeof(Uid));
            optStatus = CddDrvLS5120_PcdRequest(PICC_REQALL, pTag);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
                if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    optStep = 1;
                }
                else
                {
                    *pResultOut = eGlobalRet_OK;
                }
            }
        }

        if (optStep == 1)
        {
            optStatus = CddDrvLS5120_PcdAntiCollision(Uid, &uIdLen);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
                if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    optStep = 2;
                }
            }
        }

        if (optStep == 2)
        {
            optStatus = CddDrvLS5120_PcdSelect(Uid);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
                if(optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    memcpy(pCardUidOut, Uid, 4);
                }
            }
        }
    }

    return optStatus;
}

static uint8_t CddCardM_BullCardIdCheck(const uint8_t *pUid, uint8_t *pSectorData)
{
    uint8_t optStatus = GLOBAL_OPT_STATE_SUCCESS;
    uint8_t sectorData[16] = {0};
    uint8_t xor_uid = 0;
    uint8_t xor_data = 0;
    uint8_t index = 0;

    /* card_id(8byte) + password(3byte) + ramdom(4byte) + checkcode(1byte) */
    if (pUid == NULL || pSectorData == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        for (index = 0; index < 4; index++)
        {
            xor_uid ^= pUid[index];
            xor_data ^= pSectorData[index+1];
        }

        xor_data ^= xor_uid;
        
        if(xor_data != pSectorData[0])
        {
            optStatus = GLOBAL_OPT_STATE_FAIL;
        }
    }

    return optStatus;
}


static uint8_t CddCardM_ReadBullCardUserId(uint8_t addr, const uint8_t *pUid, uint8_t *pCardUserIdOut)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t sectorData[16] = {0};
    static uint8_t keyA[6] = {0};

    if (optStep == 0)
    {
        memset(keyA, 0, sizeof(keyA));
        CddCardM_GetBullCardSecretKeyA(pUid, (uint8_t *)BULLCARD_USERCARD_SECRET, keyA);
        optStep = 1;
    }

    if (optStep == 1)
    {
        optStatus = CddCardM_ReadSector(addr, pUid, keyA, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 2;
            }
        }
    }
    
    if (optStep == 2)
    {
        optStatus = CddCardM_BullCardIdCheck(pUid, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                memcpy(pCardUserIdOut, &sectorData[8], BULLCARD_CARDID_LEN);
            }
        }
    }

    return optStatus;
}

static uint8_t CddCardM_ReadXiaojuCardInfo(CardInfo_Struct *pInfo)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t sectorData[16] = {0};
    static uint8_t keyA[6] = {0};
    uint8_t crcCalcData[20] = {0};
	uint16_t crcValue = 0;
	uint16_t calcCrc = 0;

    if (optStep == 0)
    {
        memset(keyA, 0, sizeof(keyA));
        memcpy(keyA, (uint8_t *)c_XiaojuCardSector1KEYA, sizeof(keyA));
        optStep = 1;
    }

    if (optStep == 1)
    {
        optStatus = CddCardM_ReadSector(XIAOJU_CARD_CRCSERIAL_ADDR, pInfo->cardUid, keyA, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 2;
                memcpy(pInfo->crcSerial, sectorData, 16);
            }
        }
    }

    if (optStep == 2)
    {
        optStatus = CddDrvLS5120_PcdReadSector(XIAOJU_CARD_CRCVALUE_ADDR, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 3;
                memcpy(pInfo->crcCheck, sectorData, 16);
            }
        }
    }

    if (optStep == 3)
    {
        memcpy(crcCalcData, pInfo->crcSerial, 16);
        memcpy(crcCalcData + 16, pInfo->cardUid, 4);
		crcValue = ((uint16_t)pInfo->crcCheck[15] << 8) | pInfo->crcCheck[14];
        if (crcValue == Common_CalcCRC16(crcCalcData, 20))
        {
            memset(keyA, 0, sizeof(keyA));
            memcpy(keyA, (uint8_t *)c_XiaojuCardSector2KEYA, sizeof(keyA));
            optStep = 4;
        }
        else
        {
            optStep = 0;
            optStatus = GLOBAL_OPT_STATE_FAIL;
        }
    }

    if (optStep == 4)
    {
        optStatus = CddCardM_ReadSector(XIAOJU_CARD_CARDID_ADDR, pInfo->cardUid, keyA, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 5;
                memcpy(pInfo->cardId, sectorData, 16);
            }
        }
    }

    if (optStep == 5)
    {
        memset(keyA, 0, sizeof(keyA));
        memcpy(keyA, (uint8_t *)c_XiaojuCardSector5KEYA, sizeof(keyA));
        optStep = 6;
    }

    if (optStep == 6)
    {
        optStatus = CddCardM_ReadSector(XIAOJU_CARD_RANDOM1_ADDR, pInfo->cardUid, keyA, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 7;
                memcpy(pInfo->random1, sectorData, 16);
            }
        }
    }

    if (optStep == 7)
    {
        optStatus = CddDrvLS5120_PcdReadSector(XIAOJU_CARD_RANDOM2_ADDR, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                optStep = 8;
                memcpy(pInfo->random2, sectorData, 16);
            }
        }
    }

    if (optStep == 8)
    {
        optStatus = CddDrvLS5120_PcdReadSector(XIAOJU_CARD_RANDOM3_ADDR, sectorData);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            optStep = 0;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                memcpy(pInfo->random3, sectorData, 16);
            }
        }
    }

    return optStatus;
}

static void CddCardM_NfcInitProcess(CddCardM_Struct *pCardM)
{
    if (Common_JudgeTimeoutMs(pCardM->nfcTick, CDDCARDM_CFG_SWIPECARD_INTERVAL_TICK))
    {
        pCardM->nfcTick = Common_GetSystick();
        pCardM->initStep = 1;
    }

    if (pCardM->initStep == 1)
    {
        pCardM->initStep = 2;
        CddDrvLS5120_HardwareResetStart();
    }
    else if (pCardM->initStep == 2)
    {
        pCardM->initStep = 3;
        CddDrvLS5120_HardwareResetEnd();
    }
    else if (pCardM->initStep == 3)
    {
        pCardM->initStep = 0;
		pCardM->nfcTick = Common_GetSystick();

        if (eGlobalRet_OK == CddDrvLS5120_VersionCheck())
        {
            CddDrvLS5120_Init();
            pCardM->eNfcState = eNfcReady;
            pCardM->nfcOptStep = eOptStepReadUID;
        }
		else
		{
			pCardM->eNfcState = eNfcFault;
		}
    }
    else
    {
        pCardM->initStep = 0;
    }
	
	if (pCardM->eCardType != pCardM->eCardTypeSet)
	{
		pCardM->eCardType = pCardM->eCardTypeSet;
        CDDCARDM_CFG_InfoPrint("读卡类型：%s\r\n",c_CardTypeNameStr[pCardM->eCardType]);
	}
}

static void CddCardM_NfcReadyProcess(CddCardM_Struct *pCardM)
{
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t tagType[2] = {0};
    uint8_t tempData[16] = {0};
    uint8_t result = eGlobalRet_OK;
    uint8_t index = 0;
	
    if (pCardM->nfcOptStep == eOptStepReadUID)
    {
        optStatus = CddCardM_ReadCardUid(tagType, tempData, &result);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            pCardM->nfcOptStep = eOptStepIdle;

            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
				memset(&pCardM->stCardInfo, 0, sizeof(pCardM->stCardInfo));
				memcpy(pCardM->stCardInfo.cardUid, tempData, sizeof(pCardM->stCardInfo.cardUid));
                CDDCARDM_CFG_InfoPrint("读物理卡号成功, 物理卡号： %02X%02X%02X%02X\r\n", tempData[0],tempData[1],tempData[2],tempData[3]);

				if (pCardM->eCardType == eCddCardType_UUID)
				{
					pCardM->eCardEvent = CddCardEvent_CardIdOK;
				}
				else
				{
					pCardM->nfcOptStep = eOptStepReadUserId;
				}
            }
            else
            {
				if (result == eGlobalRet_Error)
				{
                    memset(&pCardM->stCardInfo, 0, sizeof(pCardM->stCardInfo));
					pCardM->eCardEvent = CddCardEvent_CardIdError;
					CDDCARDM_CFG_InfoPrint("读物理卡号失败！\r\n");
				}
				else
				{
					pCardM->eCardEvent = CddCardEvent_Null;
				}
            }
        }
    }

    memset(&tempData, 0, sizeof(tempData));

    if (pCardM->nfcOptStep == eOptStepReadUserId)
    {
		switch((uint8_t)pCardM->eCardType)
		{
			case eCddCardType_BullCard:
			{
				optStatus = CddCardM_ReadBullCardUserId(BULLCARD_CARDID_ADDR, pCardM->stCardInfo.cardUid, tempData);

                if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    memcpy(pCardM->stCardInfo.cardId, tempData, BULLCARD_CARDID_LEN);
                    CDDCARDM_CFG_InfoPrint("读用户卡号成功, 用户卡号: %02X%02X%02X%02X%02X%02X%02X%02X\r\n", tempData[0], tempData[1], tempData[2], 
                        tempData[3], tempData[4], tempData[5], tempData[6], tempData[7]);
                }

				break;
			}

            case eCddCardType_XiaojuCard:
            {
                optStatus = CddCardM_ReadXiaojuCardInfo(&pCardM->stCardInfo);

                if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
                {
                    CDDCARDM_CFG_InfoPrint("读用户卡号成功, 用户卡号: %.16s\r\n", pCardM->stCardInfo.cardId);
                    memcpy(tempData, pCardM->stCardInfo.cardId, XIAOJU_CARD_CARDID_LEN);
					Common_AsciiToBCD((char *)tempData, pCardM->stCardInfo.cardId, XIAOJU_CARD_CARDID_LEN);
                }

                break;
            }

			default:
			{
				optStatus = GLOBAL_OPT_STATE_FAIL;
				break;
			}
		}
		
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            pCardM->nfcOptStep = eOptStepIdle;

            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                pCardM->eCardEvent = CddCardEvent_CardIdOK;
            }
            else
            {
                memset(&pCardM->stCardInfo, 0, sizeof(CardInfo_Struct));
                pCardM->eCardEvent = CddCardEvent_CardIdError;
                CDDCARDM_CFG_InfoPrint("读卡号失败!\r\n");
            }
        }
    }

    if (pCardM->nfcOptStep == eOptStepIdle)
    {
        pCardM->eNfcState = eNfcInit;
        pCardM->nfcTick = Common_GetSystick();

        if (pCardM->eCardEvent != CddCardEvent_Null)
        {
            pCardM->eNfcState = eNfcPause;
            pCardM->eCardEventOut = pCardM->eCardEvent;

            if (pCardM->eCardEvent == CddCardEvent_CardIdOK)
            {
                pCardM->swipeCardInterval = CDDCARDM_CFG_SWIPECARD_SUCCESS_PAUSE_TICK;
            }
            else
            {
                pCardM->swipeCardInterval = CDDCARDM_CFG_SWIPECARD_FAULT_PAUSE_TICK;
            }
        }
    }
}

static void CddCardM_NfcPauseProcess(CddCardM_Struct *pCardM)
{
    if (Common_JudgeTimeoutMs(pCardM->nfcTick, pCardM->swipeCardInterval))
    {
        pCardM->eNfcState = eNfcInit;
    }
}

static void CddCardM_NfcFaultProcess(CddCardM_Struct *pCardM)
{
    if (Common_JudgeTimeoutMs(pCardM->nfcTick, CDDCARDM_CFG_SWIPECARD_FAULT_TICK))
	{
		pCardM->nfcTick = Common_GetSystick();

		if (eGlobalRet_OK != CddDrvLS5120_VersionCheck())
		{
            if (pCardM->nfcHardFaultCnt < CDDCARDM_CFG_SWIPECARD_FAULT_COUNT)
            {
                pCardM->nfcHardFaultCnt++;
            }
            else
            {
                if (pCardM->nfcHardFaultCnt == CDDCARDM_CFG_SWIPECARD_FAULT_COUNT)
                {
                    pCardM->eNfcState = eNfcInit;
                    AswErrhandle_SetErrExsitCallback(0, eErr_ReaderCommErr);
                }
            }
		}
		else
		{
			pCardM->nfcHardFaultCnt = 0;
			pCardM->eNfcState = eNfcInit;
			AswErrhandle_ResetErrExsitCallback(0, eErr_ReaderCommErr);
		}
	}
}

void CddCardM_InitMemory(void)
{
    memset(&g_stCddCardM, 0, sizeof(g_stCddCardM));
	g_stCddCardM.eCardTypeSet = eCddCardType_BullCard;
	g_stCddCardM.nfcTick = Common_GetSystick();
}
/*
1，未检测到卡或获取卡失败，按周期寻卡
2，已检测到卡，停止寻卡5秒后，重新寻卡
*/
void CddCardM_MainFunction(void)
{
    CddCardM_Struct *pCardM = &g_stCddCardM;
		
    switch((eNfcState_Enum)pCardM->eNfcState)
    {
        case eNfcInit:
        {
            CddCardM_NfcInitProcess(pCardM);
            break;
        }
        case eNfcReady:
        {
            CddCardM_NfcReadyProcess(pCardM);
            break;
        }
        case eNfcPause:
        {
            CddCardM_NfcPauseProcess(pCardM);
            break;
        }
		case eNfcFault:
		{
			CddCardM_NfcFaultProcess(pCardM);
			break;
		}
        default:
        {
            break;
        }
    }
}

GlobalRet_Enum CddCardM_SetCardType(CddCardType_Enum eType)
{
	GlobalRet_Enum ret = eGlobalRet_OK;
	
	if (eType >= eCddCardType_Count)
	{
        ret = eGlobalRet_Error;
	}
	else
	{
		g_stCddCardM.eCardTypeSet = eType;
	}

	return ret;
}

CddCardType_Enum CddCardM_GetCardType(void)
{
	return g_stCddCardM.eCardTypeSet;
}

CddCardEvent_Enum CddCardM_GetCardEvent(void)
{
    CddCardEvent_Enum cardEvent = CddCardEvent_Null;

    CddCardM_Struct *pCardM = &g_stCddCardM;
    cardEvent = pCardM->eCardEventOut;
    pCardM->eCardEventOut = CddCardEvent_Null;

    return cardEvent;
}

GlobalRet_Enum CddCardM_GetCardUid(uint8_t *pUidOut)
{
    GlobalRet_Enum ret = eGlobalRet_OK;
    CddCardM_Struct *pCardM = &g_stCddCardM;
    
    if (pUidOut == NULL)
    {
        ret = eGlobalRet_Error;
    }
    else
    {
        memcpy(pUidOut, pCardM->stCardInfo.cardUid, sizeof(pCardM->stCardInfo.cardUid));
    }

    return ret;
}

GlobalRet_Enum CddCardM_GetCardUserId(uint8_t *pId)
{
    GlobalRet_Enum ret = eGlobalRet_OK;
    CddCardM_Struct *pCardM = &g_stCddCardM;
    
    if (pId == NULL)
    {
        ret = eGlobalRet_Error;
    }
    else
    {
        memcpy(pId, pCardM->stCardInfo.cardId, 8);
    }

    return ret;
}

GlobalRet_Enum CddCardM_GetCardInfoRandom(uint8_t *pInfo)
{
    GlobalRet_Enum ret = eGlobalRet_OK;
    CddCardM_Struct *pCardM = &g_stCddCardM;
    
    if (pInfo == NULL)
    {
        ret = eGlobalRet_Error;
    }
    else
    {
        memcpy(pInfo, pCardM->stCardInfo.random1, 16);
        memcpy(pInfo + 16, pCardM->stCardInfo.random2, 16);
        memcpy(pInfo + 32, pCardM->stCardInfo.random3, 16);
    }

    return ret;
}
