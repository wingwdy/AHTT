/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "AppHeaderSummary.h"
#include "RouteHeaderSummary.h"
#include "Comminclude.h"

#include "mbsMaster.h"
#include "mbsSlave.h"
#include "card_user.h"
#include "rgb_led_scan.h"


/*
版本命名格式：manufacture.productName.productType.ModuleName_Vx.x.x_yyyy
manufacture: 表示厂商标识
productName：表示产品名称
productType：表示产品型号
ModuleName：表示模块的名称
x.x.x:	由产品经理根据项目的要求来定
yyyy:	固件发布者自定义，长度为4个字符

最大版本号为：99.99.99_0999


D3-D32B-BLE 	单相32A交流充电桩智联单元软件(单机)
D3-D32B-BLE_4G 	单相32A交流充电桩智联单元软件(联网)
*/
bfVersion  g_BootVer = {0};
bfVersion  g_HardVer = {1,0,0,0};
bfVersion  g_SoftVer = {1,0,0,1};

const stuDevInfo gc_DevInfo = { 0x01234567, 0x89ABCDEF, 0x00000002, 0x0000002C, 
	0x00000001,
	"bull.acpile.d3-d32a.scu", 
	g_HardVer,
	g_SoftVer,
	0x00000000
};

int main(void)
{
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x04000);
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	
	systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
	
	if(0U == SysTick_Config(SystemCoreClock / 1000U))
	{
		NVIC_SetPriority(SysTick_IRQn, 0x00U);
	}
	else
	{
		;
	}
	
	//任务初始化
	fgv_FuncResourceInit();

	main_var_init();
	
	RunLogModuleInit();
	MbsSlaveMod_Init();
	MbsMasterMod_Init();
	
	//任务启动运行
	RunLogTaskInit();
	RunCommunicationTaskInit();
	MbsSlaveTaskInit();
	MbsMasterTaskInit();
	// ICDealTaskInit();
	RunRGB_LED_Task();
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1)
	{
		;
	}
	
	return 0;
}
