/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Mcal_If.h"
#include "Filter.h"
// #include "Asw_Monitor.h"
// #include "Asw_Charge.h"
// #include "Asw_NFC.h"

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
	// Asw_Monitor_Task();
	// Asw_Charge_Task();
	// Asw_NFC_Task();
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
