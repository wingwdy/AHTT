/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Mcal_If.h"
#include "Mcal_Port.h"

TaskHandle_t testTaskHandle;

void Task_Test(void)
{
	uint8_t runLedState = 1;

	McalPort_WritePin(eMcalPortPinChanel_PB0_RelayEn, runLedState);

	while (1)
	{
		runLedState = !runLedState;
		McalPort_WritePin(eMcalPortPinChanel_PA1_RunLed, runLedState);
		vTaskDelay(1000);
	}
}

void TaskStartMain(void)
{
	//RGB灯语线程
	BaseType_t xResult = pdFALSE;
	xResult = xTaskCreate((void *)Task_Test, 
						"TestTaskHandle",
						256,
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
