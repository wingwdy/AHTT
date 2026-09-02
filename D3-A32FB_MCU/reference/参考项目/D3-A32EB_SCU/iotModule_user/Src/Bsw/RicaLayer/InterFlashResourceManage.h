#ifndef __INTERFLASH_RESOURCE_MANAGE_H_
#define __INTERFLASH_RESOURCE_MANAGE_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "ResourceSummaryDescribe.h"


typedef enum
{
    E_APP_FIRST_OPERATE_TYPE = 0,
    E_APP_PAGE_TYPE = 0,
    E_APP_ITEM_TYPE,
    E_APP_ADDRESS_TYPE,
	E_APP_BLOCK_ERASE_TYPE,
    E_MAX_OPERATE_TYPE_NUMBER,
}E_APP_OPERATE_TYPE_LIST;


#endif /* __INTERFLASH_RESOURCE_MANAGE_H_ */