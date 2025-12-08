/******************************************************************************
* File Name          : FlashDB_KVDB_Adapt.c
* Description        : Code for The adapter layer of KVDB
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
#include "FlashDB_KVDB_AdaptConfig.h"



/*******************************************************************************
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


/*******************************************************************************
*    Function Source Code
*******************************************************************************/
static void KVDBAdapt_GeneralKvNameByBlockID(uint16_t blockID, char *name,  uint8_t buffSize)
{
	snprintf(name, buffSize, "KV_%04d", blockID);
}

uint8_t KVDBAdapt_Init(void)
{
	KVDBAdaptConfig_Struct *pCfg = &g_stKVDBAdaptConfig;
	struct fdb_default_kv default_kv;
	uint8_t ret = FALSE;
	fdb_err_t result = FDB_NO_ERR;

	if (pCfg->pFuncCreatLock != NULL && 
		pCfg->pFuncSetLock != NULL && 
		pCfg->pFuncSetUnlock && 
		pCfg->dbName != NULL &&
		pCfg->flashPartName != NULL)
	{
		if (TRUE == pCfg->pFuncCreatLock())
		{
			fdb_kvdb_control(&pCfg->kvdb, FDB_KVDB_CTRL_SET_LOCK, pCfg->pFuncSetLock);
        	fdb_kvdb_control(&pCfg->kvdb, FDB_KVDB_CTRL_SET_UNLOCK, pCfg->pFuncSetUnlock);

			if (pCfg->default_kvs != NULL)
			{
				default_kv.kvs = pCfg->default_kvs;
				default_kv.num = pCfg->default_kvs_num;
				result = fdb_kvdb_init(&pCfg->kvdb, pCfg->dbName, pCfg->flashPartName, &default_kv, pCfg->userData);
			}
			else
			{
				result = fdb_kvdb_init(&pCfg->kvdb, pCfg->dbName, pCfg->flashPartName, NULL, pCfg->userData);
			}

			if (result == FDB_NO_ERR)
			{
				ret = TRUE;
			}
		}
	}

	return ret;
}

GlobalRet_Enum KVDBAdapt_Read(uint16_t blockID, uint8_t *pOutBuf,  uint16_t dataLen)
{
	KVDBAdaptConfig_Struct *pCfg = &g_stKVDBAdaptConfig;
	char kvName[32] = { 0 };
	struct fdb_blob blob;
	GlobalRet_Enum eRet = eGlobalRet_OK;

	KVDBAdapt_GeneralKvNameByBlockID(blockID, kvName, sizeof(kvName));
	fdb_kv_get_blob(&pCfg->kvdb, kvName, fdb_blob_make(&blob, pOutBuf, dataLen));

	if (blob.saved.len == 0)
	{
		FDB_PRINT("[KV] Key '%s' not found\r\n", kvName);
		eRet = eGlobalRet_Error;
	}
	else
	{
		if (blob.saved.len != dataLen)
		{
			FDB_PRINT("[KV] Key '%s' Data corrupted! Expected:%zu, Got:%zu, deleted the KV\r\n", kvName, dataLen, blob.saved.len);
			fdb_kv_del(&pCfg->kvdb, kvName);
			eRet = eGlobalRet_Error;
		}
	}

	return eRet;
}

GlobalRet_Enum KVDBAdapt_Write(uint16_t blockID, uint8_t *pOutBuf,  uint16_t dataLen)
{
	KVDBAdaptConfig_Struct *pCfg = &g_stKVDBAdaptConfig;
	char kvName[32] = { 0 };
	struct fdb_blob blob;

	KVDBAdapt_GeneralKvNameByBlockID(blockID, kvName, sizeof(kvName));
	fdb_kv_set_blob(&pCfg->kvdb, kvName, fdb_blob_make(&blob, pOutBuf, dataLen));
	return eGlobalRet_OK;
}



















