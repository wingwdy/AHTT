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

/*******************************************************************************
*    Macro Definition
*******************************************************************************/
#define CDDCARDM_CFG_SWIPECARD_INTERVAL_TICK    (300u)  /* 刷卡间隔 */
#define CDDCARDM_CFG_SWIPECARD_PAUSE_TICK       (5000u) /* 刷卡暂停时间 */

#define BULLCARD_USERCARD_SECRET                "bullevse666_user" /* 公牛用户卡秘钥 */
#define BULLCARD_CARDID_ADDR                    (4u) /* 公牛卡号绝对地址 */
#define BULLCARD_CARDID_LEN                     8

/*******************************************************************************
*    Enum Definition
*******************************************************************************/
typedef enum
{
    eNfcInit = 0,
    eNfcReady,
    eNfcPause,
}eNfcState_Enum;

typedef enum
{
    eOptStepIdle = 0,
    eOptStepReadUID,
    eOptStepReadUseId,
}eNfcOptStep_Enum;

/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct 
{
    uint8_t initStep;
    eNfcOptStep_Enum nfcOptStep;
	eNfcState_Enum eNfcState;
    uint32_t nfcTick; /*寻卡滴答*/
    uint8_t nfcChipVer;
    CardType_Enum eCardType;
    CardEvent_Enum eCardEvent;
    uint8_t cardUid[4];     /*卡唯一ID*/
    uint8_t cardUserid[BULLCARD_CARDID_LEN]; /*卡用户ID*/
}CddCardM_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/


/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
CddCardM_Struct gstCddCardM;

/*******************************************************************************
*    Function Source Code
*******************************************************************************/

/* 获取公牛卡块KEYA*/
uint8_t CddCardM_GetBullCardSecretKeyA(const uint8_t *pUid, const uint8_t *pBullCardSecret, uint8_t *pkeyAOut)
{
    uint8_t ret = 0;
    uint8_t i = 0;
    uint8_t encrypt_ascii[28] = {0};
    uint8_t xor_result[8] = {0};
    uint8_t md5_decrypt[16] = {0};

    if(NULL == pUid || NULL == pBullCardSecret || NULL == pkeyAOut)
    {
        ret = 1;
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

uint8_t CddCardM_ReadSector(uint8_t addr, const uint8_t *pUid, uint8_t *pSectorData)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    static uint8_t pKeyA[6] = {0};

    if (pUid == NULL || pSectorData == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        if (optStep == 0)
        {
            memset(pKeyA, 0, sizeof(pKeyA));
            CddCardM_GetBullCardSecretKeyA(pUid, (uint8_t *)BULLCARD_USERCARD_SECRET, pKeyA);
            optStep = 1;
        }

        if (optStep == 1)
        {
            optStatus = CddDrvLS5120_PcdAuthKeyA(addr, pKeyA, pUid);
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
            optStatus = CddDrvLS5120_PcdReadSector(addr, pSectorData);
            if (optStatus != GLOBAL_OPT_STATE_PROCESS)
            {
                optStep = 0;
            }
        }
    }

    return optStatus;
}

uint8_t CddCardM_ReadCardUid(uint8_t *pTag, uint8_t *pCardUidOut, uint8_t *pResultOut)
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

uint8_t CddCardM_BullCardIdCheck(const uint8_t *pUid, uint8_t *pSectorData)
{
    uint8_t optStatus = GLOBAL_OPT_STATE_SUCCESS;
    uint8_t sectorData[16] = {0};
    uint8_t xor_uid = 0;
    uint8_t xor_data = 0;

     //card_id(8byte) + password(3byte) + ramdom(4byte) + checkcode(1byte)
    if (pUid == NULL || pSectorData == NULL)
    {
        optStatus = GLOBAL_OPT_STATE_FAIL;
    }
    else
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            xor_uid ^= pUid[i];
            xor_data ^= pSectorData[i+1];
        }
        xor_data ^= xor_uid;
        if(xor_data != pSectorData[0])
        {
            optStatus = GLOBAL_OPT_STATE_FAIL;
        }
    }

    return optStatus;
}


uint8_t CddCardM_ReadCardUserId(uint8_t addr, const uint8_t *pUid, uint8_t *pCardUserIdOut)
{
    static uint8_t optStep = 0;
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t sectorData[16] = {0};

    if (optStep == 0)
    {
        optStatus = CddCardM_ReadSector(addr, pUid, sectorData);
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


void CddCardM_InitMemory(void)
{
    memset(&gstCddCardM, 0, sizeof(gstCddCardM));
    gstCddCardM.eCardType = eBullCard;
	gstCddCardM.nfcTick = Common_GetSystick();
}

void CddCardM_NfcInitProcess(CddCardM_Struct *pCardM)
{
    if (Common_GetSystick() - pCardM->nfcTick >= CDDCARDM_CFG_SWIPECARD_INTERVAL_TICK)
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
        if (eGlobalRet_OK == CddDrvLS5120_VersionCheck())
        {
            CddDrvLS5120_Init();
            pCardM->eNfcState = eNfcReady;
            pCardM->nfcOptStep = eOptStepReadUID;
        }
		else
		{
			pCardM->eCardEvent = eCardEvtHardFault;
			pCardM->eNfcState = eNfcPause;
			pCardM->nfcOptStep = eOptStepIdle;
		}
    }
    else
    {
        pCardM->initStep = 0;
    }
}

void CddCardM_NfcReadyProcess(CddCardM_Struct *pCardM)
{
    uint8_t optStatus = GLOBAL_OPT_STATE_PROCESS;
    uint8_t tagType[2] = {0};
    uint8_t tempData[16] = {0};
    uint8_t result = eGlobalRet_OK;

    if (pCardM->nfcOptStep == eOptStepReadUID)
    {
        optStatus = CddCardM_ReadCardUid(tagType, tempData, &result);
        if (optStatus != GLOBAL_OPT_STATE_PROCESS)
        {
            pCardM->nfcOptStep = eOptStepIdle;
            if (optStatus == GLOBAL_OPT_STATE_SUCCESS)
            {
                memcpy(pCardM->cardUid, tempData, 4);
				if (pCardM->eCardType == eUUIDCard)
				{
					pCardM->eCardEvent = eCardEvtCardIdOk;
				}
				else
				{
					pCardM->nfcOptStep = eOptStepReadUseId;
				}
            }
            else if(result == eGlobalRet_Error)
            {
                /* 读卡号失败 */
                pCardM->eCardEvent = eCardEvtCardIdError;
            }
        }
    }

    if (pCardM->nfcOptStep == eOptStepReadUseId)
    {
		switch((uint8_t)pCardM->eCardType)
		{
			case eBullCard:
			{
				optStatus = CddCardM_ReadCardUserId(BULLCARD_CARDID_ADDR, pCardM->cardUid, tempData);
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
                pCardM->eCardEvent = eCardEvtCardIdOk;
                memcpy(pCardM->cardUserid, tempData, BULLCARD_CARDID_LEN);
            }
            else
            {
                /* 读卡号失败 */
                pCardM->eCardEvent = eCardEvtCardIdError;
            }
        }
    }

    if (pCardM->nfcOptStep == eOptStepIdle && optStatus != GLOBAL_OPT_STATE_PROCESS)
    {
        CddDrvLS5120_HardwareResetStart();
        pCardM->eNfcState = eNfcInit;
        pCardM->nfcTick = Common_GetSystick();
        if (pCardM->eCardEvent == eCardEvtCardIdOk || pCardM->eCardEvent == eCardEvtCardIdError)
        {/* 读卡成功，读卡失败，暂停一会 */
            pCardM->eNfcState = eNfcPause;
        }
    }
}

void CddCardM_NfcPauseProcess(CddCardM_Struct *pCardM)
{
//    if (pCardM->eCardEvent == eCardEvtNone)
//    {/* 上层读取状态后，立即寻卡 */
//        pCardM->eNfcState = eNfcInit;
//    }
    if ((pCardM->eCardEvent != eCardEvtNone) && (Common_GetSystick() - pCardM->nfcTick >= CDDCARDM_CFG_SWIPECARD_PAUSE_TICK))
    {
        pCardM->eNfcState = eNfcInit;
		pCardM->eCardEvent = eCardEvtNone;
    }
    else
    {}
}
/*
1，未检测到卡或获取卡失败，按周期300ms寻卡
2，已检测到卡，停止寻卡5秒后，重新寻卡
*/
void CddCardM_MainFunction(void)
{
    CddCardM_Struct *pCardM = &gstCddCardM;
		
    switch((int)pCardM->eNfcState)
    {
        case eNfcInit:
        {
            CddCardM_NfcInitProcess(&gstCddCardM);
            break;
        }
        case eNfcReady:
        {
            CddCardM_NfcReadyProcess(&gstCddCardM);
            break;
        }
        case eNfcPause:
        {
            CddCardM_NfcPauseProcess(&gstCddCardM);
            break;
        }
        default:
        {
            break;
        }
    }
}

GlobalRet_Enum CddCardM_SetCardType(CardType_Enum eType)
{
	GlobalRet_Enum ret = eGlobalRet_OK;
	
	if (eType >= eCardTypeMax)
	{
        ret = eGlobalRet_Error;
	}
	else
	{
		if (gstCddCardM.eCardType != eType)
		{
			memset(&gstCddCardM, 0, sizeof(gstCddCardM));
			gstCddCardM.eCardType = eType;
			gstCddCardM.nfcTick = Common_GetSystick();
		}
	}

	return ret;
}

CardEvent_Enum CddCardM_GetCardEvent(void)
{
    CardEvent_Enum cardEvent = eCardEvtNone;

    CddCardM_Struct *pCardM = &gstCddCardM;
    cardEvent = pCardM->eCardEvent;
    pCardM->eCardEvent = eCardEvtNone;

    return cardEvent;
}

GlobalRet_Enum CddCardM_GetCardUid(uint8_t *pUidOut)
{
    GlobalRet_Enum ret = eGlobalRet_OK;
    CddCardM_Struct *pCardM = &gstCddCardM;
    
    if (pUidOut == NULL)
    {
        ret = eGlobalRet_Error;
    }
    else
    {
        memcpy(pUidOut, pCardM->cardUid, sizeof(pCardM->cardUid));
    }

    return ret;
}

GlobalRet_Enum CddCardM_GetCardUserId(uint8_t *pUidOut)
{
    GlobalRet_Enum ret = eGlobalRet_OK;
    CddCardM_Struct *pCardM = &gstCddCardM;
    
    if (pUidOut == NULL)
    {
        ret = eGlobalRet_Error;
    }
    else
    {
        memcpy(pUidOut, pCardM->cardUid, sizeof(pCardM->cardUid));
    }

    return ret;
}

