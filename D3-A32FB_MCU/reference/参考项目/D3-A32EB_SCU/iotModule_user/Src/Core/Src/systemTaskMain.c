#include "FreeRTOS.h"
#include "AppHeaderSummary.h"
#include "task.h"
#include "card_user.h"
#include "md5.h"
#include "card_user.h"
#include "rf_card.h"
#include "rgb_led_scan.h"
#include "Communication.h"
#include "mbsMaster.h"
#include "mbsDataUpdate.h"
#include "gd32e50x_fwdgt.h"
#include "gd32e50x.h"
#include "cmsis_os2.h"
#include "screenUart.h"
#include "protocol_ctrl.h"

/*
线程任务启动
*/

TaskHandle_t rgbTaskHandle;
TaskHandle_t netTaskHandle;
TaskHandle_t iotTaskHandle;
TaskHandle_t mbsMasterSendTaskHandle;
TaskHandle_t mbsMasterRecvTaskHandle;
TaskHandle_t HardfaultTaskHandle;
TaskHandle_t CardTaskHandle;
TaskHandle_t runLogTaskHandle;
TaskHandle_t screenTaskHandle;
TaskHandle_t impPrintfTaskHandle;
TaskHandle_t iwdgTaskHandle;

TaskHandle_t DetectTaskHandle;


void Wwdgt_Init(void)
{
    dbg_periph_enable(DBG_FWDGT_HOLD);//内核停止时（调试模式），使看门狗定时器停止工作，避免调试时触发看门狗

	//重装载值
	#define WDGT_RELOAD_VALUE   156 
	//分频系数
	#define WDGT_PRESCALER_DIV  FWDGT_PSC_DIV256

	/* enable IRC40K */
	rcu_osci_on(RCU_IRC40K);
	/* wait till IRC40K is ready */
	while(ERROR == rcu_osci_stab_wait(RCU_IRC40K));

	/* 3s */
	fwdgt_config(WDGT_RELOAD_VALUE * 5, WDGT_PRESCALER_DIV);
	fwdgt_enable();
}
void TaskIwdg(void)
{
	Wwdgt_Init();

	while(1)
	{
		osDelay(2000);
		fwdgt_counter_reload();
	}
}


void TaskStackDetect( void * pvParameters )   
{    
    UBaseType_t uxHighWaterMark;
  
    for( ;; )       
     {        
         uxHighWaterMark = uxTaskGetStackHighWaterMark(rgbTaskHandle); 
         printf("rgbTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(netTaskHandle);
         printf("netTaskHandle: %ld\r\n", uxHighWaterMark);

         uxHighWaterMark = uxTaskGetStackHighWaterMark(iotTaskHandle);
         printf("iotTaskHandle: %ld\r\n", uxHighWaterMark);

         uxHighWaterMark = uxTaskGetStackHighWaterMark(mbsMasterSendTaskHandle); 
         printf("mbsMasterSendTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(mbsMasterRecvTaskHandle); 
         printf("mbsMasterRecvTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(HardfaultTaskHandle); 
         printf("HardfaultTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(CardTaskHandle); 
         printf("CardTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(runLogTaskHandle); 
         printf("runLogTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(screenTaskHandle); 
         printf("screenTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(impPrintfTaskHandle); 
         printf("impPrintfTaskHandle: %ld\r\n", uxHighWaterMark);  

         uxHighWaterMark = uxTaskGetStackHighWaterMark(iwdgTaskHandle); 
         printf("iwdgTaskHandle: %ld\r\n", uxHighWaterMark);  
         
         vTaskDelay( 2000 );                                                                
     }    
 }

const char *HardFault_Info[] = {
	"漏电", "计量", "急停", "桩过温", "枪过温", "粘连", "拒动", "CP异常", 
	"过流", "过压", "欠压", "CP接地","PE接地", "二极管不存在", "未知故障", "未知故障",
	"未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障",
	"未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障"
	};
const char *Warning_Info[] = {
	"未接地告警", "缺相告警", "桩过温报警", "枪过温告警", "未知告警", "未知故障", "未知故障", "未知故障",
	"未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障", "未知故障"
	};

void bitGet(int bits, const char *info[], uint8_t len)
{
	int mm;
	for (mm = 0; mm < len; mm++) {
		if (bits & (1 << mm)) {
			NeedOnPrintf("%s ", info[mm]);
		}
	}
	NeedOnPrintf("\r\n\r\n");
}
void TaskPrintf( void * pvParameters )   
{    
	static GN_PLATMOD  *lpP = &sg_platmod;	//用于本文件中

	while(1)
	{
		osDelay(3000);

		for (int i = 0; i < GUN_NUM; i++) {
			if (lpP->gun[i].gunRtInfo.gunWarn.bits) {
				NeedOnPrintf("%d GunWarn = 0x%x\r\n", i, lpP->gun[i].gunRtInfo.gunWarn.bits);
				bitGet(lpP->gun[i].gunRtInfo.gunWarn.bits, Warning_Info, 16);
			}
			if (lpP->gun[i].gunRtInfo.hardfault.bits) {
				NeedOnPrintf("%d Hardfault = 0x%x\r\n", i, lpP->gun[i].gunRtInfo.hardfault.bits);
				bitGet(lpP->gun[i].gunRtInfo.hardfault.bits, HardFault_Info, 32);
			}
		}
	}
}


int TaskStartMain(void)
{	
	//RGB灯语线程
	BaseType_t xResult = pdFALSE;
	xResult = xTaskCreate((void *)RGB_LED_Main, 
						"rgbTaskHandle",
						256,
						NULL,
						PRIO_RGB_LED,
						&rgbTaskHandle
						);
	
	if(xResult != pdTRUE)
	{
		printf("rgbTaskHandle task create failure\r\n");	
	}

	//网络连接
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)runCommunicationTask, 
						"netTaskHandle",
						512,
						NULL,
						PRIO_NET_COMM,
						&netTaskHandle
						);
	
	if(xResult != pdTRUE)
	{
		printf("netTaskHandle task create failure\r\n");	
	}

	//平台连接
    xResult = pdFALSE;
	xResult = xTaskCreate((void *)runProtocolTask, 
						"iotTaskHandle",
						2048,
						NULL,
						PRIO_IOT_COMM,
						&iotTaskHandle
						);
	
	if(xResult != pdTRUE)
	{
		printf("iotTaskHandle task create failure\r\n");	
	}


	//充电板modbus 主机发送
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)MbsMasterModSendTaskMain,
				"mbsMasterSendTask",
				256,
				NULL,
				PRIO_CCU_COMM_SND,
				&mbsMasterSendTaskHandle
				);
	if(xResult != pdTRUE)
	{
		printf("mbsMasterTask send Comm task create failure\r\n");	
	}

	
	//充电板modbus 主机接收
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)MbsMasterModRecvTaskMain,
				"MbsMasterRecvComm",
				256,
				NULL,
				PRIO_CCU_COMM_RCV,
				&mbsMasterRecvTaskHandle
				);
	if(xResult != pdTRUE)
	{
		printf("MbsMaster recv Comm task create failure\r\n");	
	}

	//故障线程
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)fgv_AppEventCycleTask,
						"EventCycleTask",
						128,
						NULL,
						PRIO_EVENT,
						&HardfaultTaskHandle
						);
	
	if(xResult != pdTRUE)
	{
		printf("EventCycle task create failure\r\n");	
	}

	//刷卡线程
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)IcDealTask,
				"IcDealTask",
				256,
				NULL,
				PRIO_CARD,
				&CardTaskHandle
				);
	if(xResult != pdTRUE)
	{
		printf("IcDealTask task create failure\r\n");	
	}
	
	//Screen
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)ScrnMainTask,
						"ScrnMainTask",
						256,
						NULL,
						PRIO_SCREEN,
						&screenTaskHandle
						);
	if(xResult != pdTRUE)
	{
		bsp_printf("ScrnMainTask task create failure\r\n");	
	}


	//runlog
	xResult = pdFALSE;
	xResult = xTaskCreate(runLogTask,
						"RunLog",
						512,
						NULL,
						PRIO_RUN_LOG,
						&runLogTaskHandle
						);
	if(xResult != pdTRUE)
	{
		bsp_printf("RunLog task create failure\r\n");	
	}				
	
	//TaskPrintf
	xResult = pdFALSE;
	xResult = xTaskCreate(TaskPrintf,
						"ImpPrintf",
						256,
						NULL,
						PRIO_IMPLOG,
						&impPrintfTaskHandle
						);
	if(xResult != pdTRUE)
	{
		bsp_printf("ImpPrintf task create failure\r\n");	
	}			

	//iwdg
	xResult = pdFALSE;
	xResult = xTaskCreate((void *)TaskIwdg,
						"TaskIwdg",
						64,
						NULL,
						PRIO_IWDG,
						&iwdgTaskHandle
						);
	if(xResult != pdTRUE)
	{
		bsp_printf("TaskIwdg task create failure\r\n");	
	}			

	// 检测每个线程栈预留空间
	// xResult = pdFALSE;
	// xResult = xTaskCreate(TaskStackDetect, 
	// 					"DetectTaskHandle",
	// 					512,
	// 					NULL,
	// 					PRIO_DETECT,
	// 					&DetectTaskHandle
	// 					);
	
	// if(xResult != pdTRUE)
	// {
	// 	printf("DetectTaskHandle task create failure\r\n");	
	// }


	return 0;
}
