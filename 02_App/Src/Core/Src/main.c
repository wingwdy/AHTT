/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
// #include "AppHeaderSummary.h"

int main(void)
{
	// nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x04000);
	// nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	
	// systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
	
	// if(0U == SysTick_Config(SystemCoreClock / 1000U))
	// {
	// 	NVIC_SetPriority(SysTick_IRQn, 0x00U);
	// }
	// else
	// {
	// 	;
	// }
	
	//任务初始化
	// fgv_FuncResourceInit();

	// main_var_init();
	
	// RunLogModuleInit();
	// MbsMasterMod_Init();
	

	// TaskStartMain();
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1)
	{
		;
	}
	
	return 0;
}
