/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Mcal_If.h"

TaskHandle_t testTaskHandle;

extern void McalADC_Test(void);

void Task_Test(void)
{
	while (1)
	{
		
		vTaskDelay(1000);
		McalIf_Test();
	}
}

void TaskStartMain(void)
{
	//RGB灯语线程
	BaseType_t xResult = pdFALSE;
	xResult = xTaskCreate((void *)Task_Test, 
						"TestTaskHandle",
						512,
						NULL,
						8,
						&testTaskHandle
						);

	if(xResult != pdTRUE)
	{
//		printf("TestTaskHandle task create failure\r\n");	
	}
}







int main(void)
 {
 	McalIf_Init();
	TaskStartMain();
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	while(1)
	{}
	
	return 0;
}
