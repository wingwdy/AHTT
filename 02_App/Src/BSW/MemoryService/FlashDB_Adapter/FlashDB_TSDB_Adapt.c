/******************************************************************************
* File Name          : FlashDB_TSDB_Adapt.c
* Description        : Code for The adapter layer of TSDB
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
#include "FlashDB_TSDB_AdaptConfig.h"
#include "MS_Nvm.h"
/*******************************************************************************
*    Header File Inclusion
*******************************************************************************/




/*******************************************************************************
*    Macro Definition
*******************************************************************************/




/*******************************************************************************
*    Enum Definition
*******************************************************************************/





/*******************************************************************************
*    Typedef Definition
*******************************************************************************/
typedef struct
{
	TSDBAdaptConfig_Struct *pCfg;
	uint32_t* pInTime;
	uint32_t* pOutTime;
	fdb_tsl_status_t status;
	uint8_t *pCmpPara;
	uint16_t paraSize;
	uint8_t *pOutRecord;
	uint16_t recordSize;
	pNvmCmpFunc pCmpFunc;
	uint8_t result;
}TSDBAdaptCbArg_Struct;


/*******************************************************************************
*    Global variables Declaration
*******************************************************************************/



/*******************************************************************************
*    Static Local Functions Declaration
*******************************************************************************/
static fdb_time_t TSDBAdapt_CB_IncTime(void *user_data);
static bool TSDBAdapt_CB_SearchStatusCount(fdb_tsl_t tsl, void * arg);


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static fdb_time_t TSDBAdapt_CB_IncTime(void *user_data)
 { 
	TSDBAdaptConfig_Struct *pCfg = (TSDBAdaptConfig_Struct *)user_data;

	if (pCfg != NULL)
	{
		pCfg->time++;
	}

	return pCfg->time; 
 }

static bool TSDBAdapt_CB_SearchStatusCount(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptConfig_Struct *pCfg = (TSDBAdaptConfig_Struct *)arg;
	bool ret = FALSE;
	
	if (pCfg != NULL)
	{
		switch (tsl->status)
		{
			case FDB_TSL_WRITE:
			{
				pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount++;
				break;
			}
			case FDB_TSL_USER_STATUS1:
			{
				pCfg->staticInfo[eTSDBAdaptReportState_Reported].totalCount++;
				break;
			}
			default:
				break;
		}
	}
	else
	{
		ret = TRUE;
	}

	return ret;
}

static bool TSDBAdapt_CB_SeachUnreportedRecord(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptCbArg_Struct *pCbArg = (TSDBAdaptCbArg_Struct *)arg;
	bool ret = FALSE;
	struct fdb_blob blob = {0};

	if ((pCbArg->pInTime == NULL) || (pCbArg->pInTime[0] == tsl->time))
	{		
		if (tsl->status == FDB_TSL_WRITE)
		{
			fdb_blob_read((fdb_db_t)&pCbArg->pCfg->tsdb.parent, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, pCbArg->pOutRecord, pCbArg->recordSize)));
			pCbArg->result = TRUE;
			FDB_PRINT("[%s][%s]Search Unreported record success..., its time is [%d]\r\n", pCbArg->pCfg->flashPartName, pCbArg->pCfg->dbName, (uint32_t)tsl->time);
			if (pCbArg->pOutTime != NULL)
			{
				pCbArg->pOutTime[0] = tsl->time;
			}
			
			ret = TRUE;
		}
	}

	return ret;
}

static bool TSDBAdapt_CB_SetStatusByTime(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptCbArg_Struct *pCb_arg = (TSDBAdaptCbArg_Struct *)arg;
	bool ret = FALSE;
	
	if (tsl->time == pCb_arg->pInTime[0])
	{
		if (tsl->status < pCb_arg->status)
		{
			FDB_PRINT("[%s][%s][time: %d] the TSL status from [%d] ---> [%d]\r\n", pCb_arg->pCfg->flashPartName, pCb_arg->pCfg->dbName, 
			(uint32_t)tsl->time, (uint32_t)tsl->status, (uint32_t)pCb_arg->status);
			fdb_tsl_set_status(&pCb_arg->pCfg->tsdb, tsl, pCb_arg->status);
			pCb_arg->result = TRUE;
			ret = TRUE;
		}
		else
		{
			ret = TRUE;
		}
	}

	return ret;
}

static bool TSDBAdapt_CB_QueryLatestUnreportedRecord(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptCbArg_Struct *pCbArg = (TSDBAdaptCbArg_Struct *)arg;
	bool ret = FALSE;
	struct fdb_blob blob = {0};

	if (tsl->status == FDB_TSL_WRITE)
	{
		fdb_blob_read((fdb_db_t)&pCbArg->pCfg->tsdb.parent, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, pCbArg->pOutRecord, pCbArg->recordSize)));
		pCbArg->result = TRUE;
		FDB_PRINT("[%s][%s]Search Latest Unreported record success..., its time is [%d]\r\n", pCbArg->pCfg->flashPartName, pCbArg->pCfg->dbName, (uint32_t)tsl->time);
		if (pCbArg->pOutTime != NULL)
		{
			pCbArg->pOutTime[0] = tsl->time;
		}
		
		ret = TRUE;
	}
	
	return ret;
}

static bool TSDBAdapt_CB_QueryRecordByTime(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptCbArg_Struct *pCbArg = (TSDBAdaptCbArg_Struct *)arg;
	bool ret = FALSE;
	struct fdb_blob blob = {0};

	if ((pCbArg->pInTime[0] == tsl->time))
	{
		fdb_blob_read((fdb_db_t)&pCbArg->pCfg->tsdb.parent, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, pCbArg->pOutRecord, pCbArg->recordSize)));
		pCbArg->result = TRUE;
		FDB_PRINT("[%s][%s]Search record success by time..., its time is [%d]\r\n", pCbArg->pCfg->flashPartName, pCbArg->pCfg->dbName, (uint32_t)tsl->time);
		ret = TRUE;
	}
	
	return ret;
}

static bool TSDBAdapt_CB_QueryRecordByExternal(fdb_tsl_t tsl, void * arg)
{
	TSDBAdaptCbArg_Struct *pCbArg = (TSDBAdaptCbArg_Struct *)arg;
	bool ret = FALSE;
	struct fdb_blob blob = {0};

	if (pCbArg->pCmpPara != NULL && pCbArg->paraSize != 0 && pCbArg->pCmpFunc != NULL &&
	pCbArg->pOutRecord != NULL && pCbArg->recordSize != 0)
	{
		fdb_blob_read((fdb_db_t)&pCbArg->pCfg->tsdb.parent, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, pCbArg->pOutRecord, pCbArg->recordSize)));
		pCbArg->result = pCbArg->pCmpFunc(pCbArg->pOutRecord, pCbArg->pCmpPara, pCbArg->paraSize);

		if (pCbArg->result == TRUE)
		{
			FDB_PRINT("[%s][%s]Search record success by External..., its time is [%d]\r\n", pCbArg->pCfg->flashPartName, pCbArg->pCfg->dbName, (uint32_t)tsl->time);
			ret = TRUE;
		}
	}

	return ret;
}


static uint8_t TSDFAdapt_ConfigChannel(TSDBAdaptConfig_Struct *pCfg)
{
    uint8_t ret = FALSE;

    if (pCfg->pFuncCreatLock != NULL && pCfg->pFuncSetLock != NULL && 
        pCfg->pFuncSetUnlock != NULL && pCfg->dbName != NULL && pCfg->flashPartName != NULL)
    {
		if (TRUE == pCfg->pFuncCreatLock())
		{
			fdb_tsdb_control(&pCfg->tsdb, FDB_TSDB_CTRL_SET_LOCK,   pCfg->pFuncSetLock);
        	fdb_tsdb_control(&pCfg->tsdb, FDB_TSDB_CTRL_SET_UNLOCK, pCfg->pFuncSetUnlock);

            if (FDB_NO_ERR == fdb_tsdb_init(&pCfg->tsdb, pCfg->dbName, pCfg->flashPartName, TSDBAdapt_CB_IncTime, pCfg->maxLen, NULL))
            {
                ret = TRUE;
                fdb_tsl_iter(&pCfg->tsdb, TSDBAdapt_CB_SearchStatusCount, pCfg);
                fdb_tsdb_control(&pCfg->tsdb, FDB_TSDB_CTRL_GET_LAST_TIME, &pCfg->time);
            }
        }
    }

    return ret;
}

static uint8_t TSDBAdapt_SetStatusByTime(TSDBAdaptChannel_Enum eCh, uint32_t time, fdb_tsl_status_t status)
{
	TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
	TSDBAdaptCbArg_Struct cb_arg = { 0 };
	uint8_t ret = FALSE;
	
    cb_arg.pCfg = pCfg ;
    cb_arg.status = status;
    cb_arg.pInTime = &time;
    fdb_tsl_iter_by_time(&pCfg->tsdb, time, time, TSDBAdapt_CB_SetStatusByTime, &cb_arg);

    if (cb_arg.result == TRUE)
    {
        ret = TRUE;
    }
	
	return ret;
}

GlobalRet_Enum TSDBAdapt_GetlatestUnreportRecord(TSDBAdaptChannel_Enum eCh, uint8_t *pOutRecord, uint16_t recordSize)
{
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
    PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);

    return eGlobalRet_OK;
}

GlobalRet_Enum TSDBAdapt_CleanRecordDB(uint16_t ch)
{
    TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
    PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);

    fdb_tsl_clean(&pCfg->tsdb);
    return eGlobalRet_OK;
}

uint32_t TSDBAdapt_QueryUnreportedRecordCount(uint16_t ch)
{
    TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
    PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, 0);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, 0);

    return pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount;
}

uint32_t TSDBAdapt_QueryTotalRecordCount(uint16_t ch)
{
    TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
    PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, 0);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, 0);

    return (pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount + \
    pCfg->staticInfo[eTSDBAdaptReportState_Reported].totalCount);
}

GlobalRet_Enum TSDBAdapt_InsertRecord(uint16_t ch, uint8_t *pInBuf, uint16_t dataLen)
{
    TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
    GlobalRet_Enum eRet = eGlobalRet_Error;
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
    PARA_ASSERT_RET((eCh < eTSDBAdaptChannel_Count) && (pInBuf != NULL) && (dataLen <= pCfg->maxLen), eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);
    fdb_err_t fdb_err;
	struct fdb_blob blob = {0};

    fdb_err = fdb_tsl_append(&pCfg->tsdb, fdb_blob_make(&blob, pInBuf, dataLen));

    if (fdb_err == FDB_NO_ERR)
    {
        pCfg->pFuncSetLock();
        pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount++;
        pCfg->pFuncSetUnlock();
		FDB_PRINT("[%s][%s][time: %d] save record Success....\r\n", pCfg->flashPartName, pCfg->dbName, pCfg->time);
        eRet = eGlobalRet_OK;
    }
	else
	{
		FDB_PRINT("[%s][%s][time: %d] save record Failed....\r\n", pCfg->flashPartName, pCfg->dbName, pCfg->time);
	}

    return eRet;
}

GlobalRet_Enum TSDBAdapt_SetRecordReportSuccess(uint16_t ch, uint32_t time)
{
    TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
    GlobalRet_Enum eRet = eGlobalRet_Error;
    TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];

    PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);
	PARA_ASSERT_RET(pCfg->time > 0 && time <= pCfg->time, eGlobalRet_NotEnoughData);

    if (TSDBAdapt_SetStatusByTime(eCh, time, FDB_TSL_USER_STATUS1))
	{
		if (pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount > 0)
		{
			pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount--;
			eRet = eGlobalRet_OK;
		}
	}

	return eRet;
}

GlobalRet_Enum TSDBAdapt_QueryLatestUnreportedRecord(uint16_t ch, uint8_t *pOutBuf, uint16_t dataLen, uint32_t *pTime)
{
	TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
	GlobalRet_Enum eRet = eGlobalRet_Error;
	TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
	TSDBAdaptCbArg_Struct cb_arg = { 0 };

	PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);
	PARA_ASSERT_RET(pOutBuf != NULL && dataLen != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);
	PARA_ASSERT_RET(pCfg->staticInfo[eTSDBAdaptReportState_unReported].totalCount > 0, eGlobalRet_NotEnoughData);

    cb_arg.pCfg = pCfg;
	cb_arg.pOutTime = pTime;
	cb_arg.pOutRecord = pOutBuf;
	cb_arg.recordSize = dataLen;
	fdb_tsl_iter_reverse(&pCfg->tsdb, TSDBAdapt_CB_QueryLatestUnreportedRecord, &cb_arg); 

	if (cb_arg.result == TRUE)
	{
		eRet = eGlobalRet_OK;
	}

	return eRet;
}

GlobalRet_Enum TSDBAdapt_QueryRecordByTime(uint16_t ch, uint8_t *pOutBuf, uint16_t dataLen, uint32_t time)
{
	TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
	GlobalRet_Enum eRet = eGlobalRet_Error;
	TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
	TSDBAdaptCbArg_Struct cb_arg = { 0 };

	PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);
	PARA_ASSERT_RET(pOutBuf != NULL && dataLen != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);
	PARA_ASSERT_RET(pCfg->time > 0 && time <= pCfg->time, eGlobalRet_NotEnoughData);

	cb_arg.pCfg = pCfg;
	cb_arg.pInTime = &time;
	cb_arg.pOutRecord = pOutBuf;
	cb_arg.recordSize = dataLen;
	
	fdb_tsl_iter_by_time(&pCfg->tsdb, time, time, TSDBAdapt_CB_QueryRecordByTime, &cb_arg);

	if (cb_arg.result == TRUE)
	{
		eRet = eGlobalRet_OK;
	}

	return eRet;
}

GlobalRet_Enum TSDBAdapt_QueryRecordByExternal(uint16_t ch, uint8_t *para, uint16_t paraSize,
    pNvmCmpFunc pCmpFunc, uint8_t *pInBuf, uint16_t dataLen)
{
	TSDBAdaptChannel_Enum eCh = (TSDBAdaptChannel_Enum)ch;
	GlobalRet_Enum eRet = eGlobalRet_Error;
	TSDBAdaptConfig_Struct *pCfg = &g_stTSDBAdaptConfigTable[eCh];
	TSDBAdaptCbArg_Struct cb_arg = { 0 };

	PARA_ASSERT_RET(eCh < eTSDBAdaptChannel_Count, eGlobalRet_ParaInvalid);
	PARA_ASSERT_RET(pCmpFunc != NULL, eGlobalRet_ParaInvalid);
	PARA_ASSERT_RET(para != NULL && paraSize != 0, eGlobalRet_ParaInvalid);
	PARA_ASSERT_RET(pInBuf != NULL && dataLen != 0, eGlobalRet_ParaInvalid);
    PARA_ASSERT_RET(pCfg->initFlag == TRUE, eGlobalRet_NotInit);
	PARA_ASSERT_RET(pCfg->time > 0, eGlobalRet_NotEnoughData);

	cb_arg.pCfg = pCfg;
	cb_arg.pOutRecord = pInBuf;
	cb_arg.recordSize = dataLen;
	cb_arg.pCmpFunc = pCmpFunc;
	cb_arg.pCmpPara = para;
	cb_arg.paraSize = paraSize;
	fdb_tsl_iter_reverse(&pCfg->tsdb, TSDBAdapt_CB_QueryRecordByExternal, &cb_arg);

	if (cb_arg.result == TRUE)
	{
		eRet = eGlobalRet_OK;
	}

	return eRet;
}

uint8_t TSDBAdapt_Init(void)
{
	TSDBAdaptConfig_Struct *pCfg = NULL;
	uint8_t index = 0;
    uint8_t ret = TRUE;

    for (index = 0; index < ARRAY_SIZE(g_stTSDBAdaptConfigTable); index++)
    {
        pCfg = &g_stTSDBAdaptConfigTable[index];

        pCfg->initFlag = TSDFAdapt_ConfigChannel(pCfg);

        if (pCfg->initFlag == FALSE)
        {
            ret = FALSE;
            break;
        }
    }

    return ret;
}






















