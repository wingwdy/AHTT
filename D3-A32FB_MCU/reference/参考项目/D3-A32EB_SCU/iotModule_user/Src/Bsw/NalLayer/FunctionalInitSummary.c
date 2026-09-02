#include "RouteHeaderSummary.h"


void fgv_FuncResourceInit(void)
{
    fgv_AppGpioInit();
    fgu32_AppUartInit();
	//delay 10ms wait extern chip ready
	fgv_AppInfoStoreInit();
}


void fgv_AppSoftwareReset(void)
{
    fgv_ResourceSoftwareReset();
}

