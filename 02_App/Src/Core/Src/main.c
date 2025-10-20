/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Mcal_If.h"


int main(void)
{
	McalIf_Init();
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1)
	{
		;
	}
	
	return 0;
}
