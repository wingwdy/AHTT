/***********************************************************************************
 * 文 件 名  : touch.c
 * 版 本 号  : V1.0
 * 负 责 人  : WEEN
 * 创建日期  : 2021-5-1
 * 文件描述  : 触摸处理函数
 * 版权说明  : Copyright (c) 2021-2025  公牛集团
 * 函数列表  :
 * 其    他  :
 * 修改日志  :
***********************************************************************************/

#include "AppHeaderSummary.h"
#include "rgb_led.h"
#include "rgb_led_scan.h"
#include "card_user.h"

/* pwm timer*/
#define LED_PWM_TIMER	TIMER1
/* pwm timer period*/
#define TIMER_PERIOD_CNT	20	

#if LED_DRIVER_IO == 1
    #define RGB_LED_HIGH      0xC0
    #define RGB_LED_LOW       0x80
#else
    #define RGB_LED_HIGH      15
    #define RGB_LED_LOW       6
#endif

/* rgb灯数据缓存区 */
uint8_t pixelBuffer[Led_MAX_NUM][24] = {0};
/* rgb灯数据总长度 */
#define RGB_BUFLEN  (Led_MAX_NUM * 24)



void dma_config(void)
{
    dma_parameter_struct dma_init_struct;

    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);

    /* initialize DMA channel5 */
    dma_deinit(DMA0,DMA_CH4);

    /* DMA channel5 initialize */
	// #define TIMER1_CH0CV  ((uint32_t)0x40000034)
    // dma_init_struct.periph_addr = (uint32_t)TIMER1_CH0CV;
    dma_init_struct.periph_addr = (uint32_t)(&TIMER_DMATB(TIMER1));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_addr = (uint32_t)pixelBuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.number = RGB_BUFLEN;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH4);
    // dma_circulation_enable(DMA0, DMA_CH4);

	dma_memory_to_memory_disable(DMA0, DMA_CH4);
    /* enable DMA channel5 */
    // dma_channel_enable(DMA0,DMA_CH4);

	dma_interrupt_enable(DMA0,DMA_CH4,DMA_INT_FTF);  //DMA中断使能
    
    nvic_irq_enable(DMA0_Channel4_IRQn, 2, 1);
}

/*****************************************************************************
 * 函 数 名  : Timer_Init
 * 负 责 人  : WEEN
 * 创建日期  : 2021年4月1日
 * 函数功能  : Timer初始化
 * 输入参数  : 无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  :
*****************************************************************************/
void LED_Timer_Init(void)
{
	dma_config();

	//90k
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_AF);

	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    gpio_pin_remap_config(GPIO_TIMER1_FULL_REMAP, ENABLE);
    // gpio_pin_remap_config(GPIO_TIMER1_PARTIAL_REMAP1, ENABLE);

	/* configure PA12(TIMER0_ETI) as alternate function */
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

	timer_oc_parameter_struct timer_ocintpara;
	timer_parameter_struct timer_initpara;

	rcu_periph_clock_enable(RCU_TIMER1);
	timer_deinit(LED_PWM_TIMER);

	/* TIMER configuration */
	timer_initpara.prescaler         = 10;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = TIMER_PERIOD_CNT;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(LED_PWM_TIMER,&timer_initpara);

	/* configurate CH0 in PWM mode0 */
	timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
	timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
	timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
	timer_channel_output_config(LED_PWM_TIMER, TIMER_CH_0, &timer_ocintpara);

	timer_channel_output_pulse_value_config(LED_PWM_TIMER, TIMER_CH_0, 0);
	timer_counter_value_config(LED_PWM_TIMER,0);//预置定时器数据,以便触发峰值AD采样
	timer_channel_output_mode_config(LED_PWM_TIMER, TIMER_CH_0, TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(LED_PWM_TIMER, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// timer_channel_output_shadow_config(LED_PWM_TIMER, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);

    /* channel DMA request source selection */
    timer_channel_dma_request_source_select(LED_PWM_TIMER,TIMER_DMAREQUEST_UPDATEEVENT);
    /* configure the TIMER DMA transfer */ 
    timer_dma_transfer_config(LED_PWM_TIMER,TIMER_DMACFG_DMATA_CH0CV,TIMER_DMACFG_DMATC_1TRANSFER);
    /* TIMER0 update DMA request enable */
    timer_dma_enable(LED_PWM_TIMER,TIMER_DMA_CH0D);

	// timer_primary_output_config(LED_PWM_TIMER,ENABLE);
	/* auto-reload preload enable */
	timer_auto_reload_shadow_enable(LED_PWM_TIMER);
	
	timer_enable(LED_PWM_TIMER);
}


void DMA0_Channel4_IRQHandler(void)
{
	if(dma_interrupt_flag_get(DMA0,DMA_CH4,DMA_INT_FLAG_FTF))
    {
	    timer_disable(TIMER1);
		timer_channel_output_pulse_value_config(LED_PWM_TIMER, TIMER_CH_0, 0);
        dma_interrupt_flag_clear(DMA0,DMA_CH4,DMA_INT_FLAG_FTF);//清除中断标志     
        dma_channel_disable(DMA0, DMA_CH4);
        dma_transfer_number_config(DMA0, DMA_CH4, RGB_BUFLEN);//传输DMA数据
    }
}




/************************************************
 * 灯缓存数据传输，最后一步调用
 ************************************************/
void ws281x_BufferTransfer(uint8_t* buf, int len)
{
#if LED_DRIVER_IO == 1
	led_io_sendDataBuf(buf, len);
#else
	dma_channel_enable(DMA0,DMA_CH4);
	timer_enable(TIMER1);
#endif

}


/************************************************
 * 根据颜色更新需要传输的数据
 ************************************************/
void ws281x_BufferUpdate(uint32_t *RGBcolor, uint8_t n)
{
	if(n > Led_MAX_NUM) {
		return;
	}

  memset(pixelBuffer,RGB_LED_LOW,sizeof(pixelBuffer));

  for (int j = 0; j < n; j++) {
    uint8_t _r = RGBcolor[j] >> 16;
    uint8_t _g = RGBcolor[j] >> 8;
    uint8_t _b = RGBcolor[j];
    uint32_t GRBcolor = (_g << 16) | (_r << 8) | (_b << 0);

    for(int i = 0; i < 24; ++i)
    {
      pixelBuffer[j][i] = (((GRBcolor << i) & 0X800000) ? RGB_LED_HIGH : RGB_LED_LOW);
    } 
  }
}

