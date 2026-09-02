#include "AppCfg.h"
#include "AppStorage.h"
#include "cost.h"
#include "rgb_led.h"
#include "mbsMaster.h"

void main_var_init()
{
	Cost_init();		//cost计费信息初始化
	load_AllParam();	//读取flash参数
}

void Reboot_System(uint8_t n)
{
	//0重启所有，1重启A，2重启B
	if (n == 0) {
		printf("All reboot...\r\n");

		printf("charge board reboot...\r\n");
		fgv_CtrlPileOpr(E_DEV_CTRL_CMD_REBOOT);
		//控制主板重启完成后，网络单元重启
		osDelay(100);
		printf("network board reboot...\r\n");
		NVIC_SystemReset();
	} else if (n == 1) {
		printf("charge board reboot...\r\n");
		fgv_CtrlPileOpr(E_DEV_CTRL_CMD_REBOOT);

	} else {
		printf("network board reboot...\r\n");
		NVIC_SystemReset();
	}
	
}