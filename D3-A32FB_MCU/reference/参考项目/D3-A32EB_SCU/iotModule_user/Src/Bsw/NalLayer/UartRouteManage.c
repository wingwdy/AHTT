#include "UartRouteManage.h"
#include <stdio.h>

static const uint32_t sg32_UartIDtoUARTxPERIH[E_MAX_UART_NUMBER] = {USART0,USART1,USART2,UART3,UART4,USART5};

static const STRU_UART_CFG sg_UartCfg[E_MAX_UART_NUMBER] =
{
	{//USART0 (DEBUG)
		.u8_ManageChannel = 0,
		.u8_Enable = 1,
		.u32_UARTx_PERIPH = USART0,
		 .u32_UARTx_RCU = RCU_USART0,
	    .u32_UARTx_IRQ = USART0_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOC,
		.u32_TX_GPIOx = GPIOA,
		.u16_TX_GPIO_Pin = GPIO_PIN_9,
		.u32_RX_RCU_GPIOx = RCU_GPIOA,
		.u32_RX_GPIOx = GPIOA,
		.u16_RX_GPIO_Pin = GPIO_PIN_10,
		.u32_BaudRate = 115200,	
	},
	{//USART1 (CCU)
		.u8_ManageChannel = 1,
		.u8_Enable = 1,
		.u32_UARTx_PERIPH = USART1,
		.u32_UARTx_RCU = RCU_USART1,
	    .u32_UARTx_IRQ = USART1_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOA,
		.u32_TX_GPIOx = GPIOA,
		.u16_TX_GPIO_Pin = GPIO_PIN_2,
		.u32_RX_RCU_GPIOx = RCU_GPIOA,
		.u32_RX_GPIOx = GPIOA,
		.u16_RX_GPIO_Pin = GPIO_PIN_3,
		.u32_BaudRate = 115200,
	},
	{//USART2 (4G)
		.u8_ManageChannel = 2,
		.u8_Enable = 1,
		.u32_UARTx_PERIPH = USART2,
		.u32_UARTx_RCU = RCU_USART2,
	    .u32_UARTx_IRQ = USART2_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOB,
		.u32_TX_GPIOx = GPIOB,
		.u16_TX_GPIO_Pin = GPIO_PIN_10,
		.u32_RX_RCU_GPIOx = RCU_GPIOB,
		.u32_RX_GPIOx = GPIOB,
		.u16_RX_GPIO_Pin = GPIO_PIN_11,
		.u32_BaudRate = 115200,
	},
	{//UART3 (HMI)
		.u8_ManageChannel = 3,
		.u8_Enable = 0,
		.u32_UARTx_PERIPH = UART3,
		.u32_UARTx_RCU = RCU_UART3,
	    .u32_UARTx_IRQ = UART3_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOC,
		.u32_TX_GPIOx = GPIOC,
		.u16_TX_GPIO_Pin = GPIO_PIN_10,
		.u32_RX_RCU_GPIOx = RCU_GPIOC,
		.u32_RX_GPIOx = GPIOC,
		.u16_RX_GPIO_Pin = GPIO_PIN_11,
		.u32_BaudRate = 115200,	
	},
	{//UART4 (屏幕)
		.u8_ManageChannel = 4,
		.u8_Enable = 1,
		.u32_UARTx_PERIPH = UART4,
		.u32_UARTx_RCU = RCU_UART4,
	    .u32_UARTx_IRQ = UART4_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOC,
		.u32_TX_GPIOx = GPIOC,
		.u16_TX_GPIO_Pin = GPIO_PIN_12,
		.u32_RX_RCU_GPIOx = RCU_GPIOD,
		.u32_RX_GPIOx = GPIOD,
		.u16_RX_GPIO_Pin = GPIO_PIN_2,
		.u32_BaudRate = 38400,		
	},
	{//USART5 (DEBUG)
		.u8_ManageChannel = 5,
		.u8_Enable = 0,
		.u32_UARTx_PERIPH = USART0,
		.u32_UARTx_RCU = RCU_USART0,
	    .u32_UARTx_IRQ = USART0_IRQn,
		.u32_TX_RCU_GPIOx = RCU_GPIOA,
		.u32_TX_GPIOx = GPIOA,
		.u16_TX_GPIO_Pin = GPIO_PIN_9,
		.u32_RX_RCU_GPIOx = RCU_GPIOA,
		.u32_RX_GPIOx = GPIOA,
		.u16_RX_GPIO_Pin = GPIO_PIN_10,
		.u32_BaudRate = 115200,
	}		
};

static uint8_t sgu8_TxBuf1[UART1_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf1[UART1_RX_BUF_SIZE];		// 接收缓冲区
static uint8_t sgu8_TxBuf2[UART2_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf2[UART2_RX_BUF_SIZE];		// 接收缓冲区
static uint8_t sgu8_TxBuf3[UART3_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf3[UART3_RX_BUF_SIZE];		// 接收缓冲区
static uint8_t sgu8_TxBuf4[UART4_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf4[UART4_RX_BUF_SIZE];		// 接收缓冲区
static uint8_t sgu8_TxBuf5[UART5_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf5[UART5_RX_BUF_SIZE];		// 接收缓冲区
static uint8_t sgu8_TxBuf6[UART6_TX_BUF_SIZE];		// 发送缓冲区
static uint8_t sgu8_RxBuf6[UART6_RX_BUF_SIZE];		// 接收缓冲区

static uint8_t sgu8_RxCompFlag[E_MAX_UART_NUMBER] = {0};

/* 串口数据结构初始化 */
static STRU_UART_MANAGE sg_AppUartManage[] = 
{
	{
		.pu8_TxBuf     = sgu8_TxBuf1,
		.pu8_RxBuf     = sgu8_RxBuf1,
		.u16_TxBufSize = sizeof(sgu8_TxBuf1),
		.u16_RxBufSize = sizeof(sgu8_RxBuf1),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	},
	{
		.pu8_TxBuf     = sgu8_TxBuf2,
		.pu8_RxBuf     = sgu8_RxBuf2,
		.u16_TxBufSize = sizeof(sgu8_TxBuf2),
		.u16_RxBufSize = sizeof(sgu8_RxBuf2),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	},
	{
		.pu8_TxBuf     = sgu8_TxBuf3,
		.pu8_RxBuf     = sgu8_RxBuf3,
		.u16_TxBufSize = sizeof(sgu8_TxBuf3),
		.u16_RxBufSize = sizeof(sgu8_RxBuf3),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	},
	{
		.pu8_TxBuf     = sgu8_TxBuf4,
		.pu8_RxBuf     = sgu8_RxBuf4,
		.u16_TxBufSize = sizeof(sgu8_TxBuf4),
		.u16_RxBufSize = sizeof(sgu8_RxBuf4),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	},
	{
		.pu8_TxBuf     = sgu8_TxBuf5,
		.pu8_RxBuf     = sgu8_RxBuf5,
		.u16_TxBufSize = sizeof(sgu8_TxBuf5),
		.u16_RxBufSize = sizeof(sgu8_RxBuf5),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	},
	{
		.pu8_TxBuf     = sgu8_TxBuf6,
		.pu8_RxBuf     = sgu8_RxBuf6,
		.u16_TxBufSize = sizeof(sgu8_TxBuf6),
		.u16_RxBufSize = sizeof(sgu8_RxBuf6),
		.u16_TxRead    = 0,
		.u16_TxLen     = 0,
		.u16_RxHead    = 0,
		.u16_RxLen     = 0,
		.u8_UartInited = 0,
		.u8_UartTxMode  = E_TX_MODE_DIR,
		.u8_BufTC      = 0,
		.u32_DMA_FLAG_TCIFx = 0,
	}	
};


//ccu dma
#define CCU_UART_DMAx               DMA0
#define CCU_UART_DMAx_RX_CHANNEL    DMA_CH5
#define CCU_UART_DMAx_TX_CHANNEL    DMA_CH6
#define CCU_UART_DMAx_PHRIPH_ADDR   0x40004404

#define CCU_UART_DMAx_RX_LEN              256
#define CCU_UART_DMAx_TX_LEN              256
uint8_t s_uartCcuRx_dmaBuf[CCU_UART_DMAx_RX_LEN] = {0};
uint8_t s_uartCcuTx_dmaBuf[CCU_UART_DMAx_TX_LEN] = {0};

//gprs dma
#define GPRS_UART_DMAx               DMA0
#define GPRS_UART_DMAx_RX_CHANNEL    DMA_CH2
#define GPRS_UART_DMAx_TX_CHANNEL    DMA_CH1
#define GPRS_UART_DMAx_PHRIPH_ADDR   0x40004804

#define GPRS_UART_DMAx_RX_LEN              1536
#define GPRS_UART_DMAx_TX_LEN              1024
uint8_t s_uartGPRSRx_dmaBuf[GPRS_UART_DMAx_RX_LEN] = {0};
uint8_t s_uartGPRSTx_dmaBuf[GPRS_UART_DMAx_TX_LEN] = {0};



void CCU_UartRxDmaConfig()
{
	STRU_UART_MANAGE *pstu_UartManage = &sg_AppUartManage[sg_UartCfg[E_UART1_INDEX].u8_ManageChannel];

    dma_parameter_struct stDmaParam;

    rcu_periph_clock_enable(RCU_DMA0);
    dma_deinit(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);
    
    stDmaParam.direction = DMA_PERIPHERAL_TO_MEMORY;
    stDmaParam.memory_addr = (uint32_t)s_uartCcuRx_dmaBuf;
    stDmaParam.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    stDmaParam.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    stDmaParam.number = CCU_UART_DMAx_RX_LEN;
    stDmaParam.periph_addr = CCU_UART_DMAx_PHRIPH_ADDR;
    stDmaParam.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    stDmaParam.priority = DMA_PRIORITY_MEDIUM;
    dma_init(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL, &stDmaParam);

    dma_circulation_enable(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);
    
    dma_channel_enable(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);

    usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
}

void CCU_UartTxDmaConfig()
{
	STRU_UART_MANAGE *pstu_UartManage = &sg_AppUartManage[sg_UartCfg[E_UART1_INDEX].u8_ManageChannel];

    dma_parameter_struct stDmaParam;

    rcu_periph_clock_enable(RCU_DMA0);
    dma_deinit(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);
    
    stDmaParam.direction = DMA_MEMORY_TO_PERIPHERAL;
    stDmaParam.memory_addr = (uint32_t)s_uartCcuTx_dmaBuf;
    stDmaParam.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    stDmaParam.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    stDmaParam.number = CCU_UART_DMAx_TX_LEN;
    stDmaParam.periph_addr = CCU_UART_DMAx_PHRIPH_ADDR;
    stDmaParam.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    stDmaParam.priority = DMA_PRIORITY_MEDIUM;
    dma_init(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL, &stDmaParam);

    dma_circulation_disable(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);

    dma_memory_to_memory_disable(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);

    dma_channel_disable(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);
}

E_UART_ERR fge_CCU_UartDMASendRoute(uint8_t *pu8_Buf, uint16_t u16_SndLen)
{
    if (u16_SndLen > CCU_UART_DMAx_TX_LEN) {
        return E_UART_BUF_LEN_ERR;
    }
    memcpy(s_uartCcuTx_dmaBuf, pu8_Buf, u16_SndLen);

    
    dma_channel_disable(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);

    //设置要发送数据的内存地址
	dma_memory_address_config(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL,(uint32_t)s_uartCcuTx_dmaBuf);
    //设置要发送数据长度
	dma_transfer_number_config(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL,u16_SndLen);
    
    dma_channel_enable(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL);

  	usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);//使能串口DMA发送

	while(RESET == dma_flag_get(CCU_UART_DMAx, CCU_UART_DMAx_TX_CHANNEL, DMA_FLAG_FTF))
    {

    }
    return E_UART_NO_ERR;
}


void GPRS_UartRxDmaConfig()
{
	STRU_UART_MANAGE *pstu_UartManage = &sg_AppUartManage[sg_UartCfg[E_UART2_INDEX].u8_ManageChannel];

    dma_parameter_struct stDmaParam;

    rcu_periph_clock_enable(RCU_DMA0);
    dma_deinit(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);
    
    stDmaParam.direction = DMA_PERIPHERAL_TO_MEMORY;
    stDmaParam.memory_addr = (uint32_t)s_uartGPRSRx_dmaBuf;
    stDmaParam.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    stDmaParam.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    stDmaParam.number = GPRS_UART_DMAx_RX_LEN;
    stDmaParam.periph_addr = GPRS_UART_DMAx_PHRIPH_ADDR;
    stDmaParam.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    stDmaParam.priority = DMA_PRIORITY_MEDIUM;
    dma_init(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL, &stDmaParam);

    dma_circulation_enable(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);
    
    dma_channel_enable(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);

    usart_dma_receive_config(USART2, USART_RECEIVE_DMA_ENABLE);
}

void GPRS_UartTxDmaConfig()
{
	STRU_UART_MANAGE *pstu_UartManage = &sg_AppUartManage[sg_UartCfg[E_UART2_INDEX].u8_ManageChannel];

    dma_parameter_struct stDmaParam;

    rcu_periph_clock_enable(RCU_DMA0);
    dma_deinit(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);
    
    stDmaParam.direction = DMA_MEMORY_TO_PERIPHERAL;
    stDmaParam.memory_addr = (uint32_t)s_uartGPRSTx_dmaBuf;
    stDmaParam.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    stDmaParam.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    stDmaParam.number = GPRS_UART_DMAx_TX_LEN;
    stDmaParam.periph_addr = GPRS_UART_DMAx_PHRIPH_ADDR;
    stDmaParam.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    stDmaParam.priority = DMA_PRIORITY_MEDIUM;
    dma_init(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL, &stDmaParam);

    dma_circulation_disable(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);

    dma_memory_to_memory_disable(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);

    dma_channel_disable(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);
}

E_UART_ERR fge_GPRS_UartDMASendRoute(uint8_t *pu8_Buf, uint16_t u16_SndLen)
{
    if (u16_SndLen > GPRS_UART_DMAx_TX_LEN) {
        return E_UART_BUF_LEN_ERR;
    }
    memcpy(s_uartGPRSTx_dmaBuf, pu8_Buf, u16_SndLen);

    
    dma_channel_disable(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);

    //设置要发送数据的内存地址
	dma_memory_address_config(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL,(uint32_t)s_uartGPRSTx_dmaBuf);
    //设置要发送数据长度
	dma_transfer_number_config(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL,u16_SndLen);
    
    dma_channel_enable(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL);

  	usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);//使能串口DMA发送

	while(RESET == dma_flag_get(GPRS_UART_DMAx, GPRS_UART_DMAx_TX_CHANNEL, DMA_FLAG_FTF))
    {

    }
    return E_UART_NO_ERR;
}





static void BspUartInit(void)
{
	for(uint8_t i = E_FIRST_UART_INDEX; i < E_MAX_UART_NUMBER; i++)
	{
		if(sg_UartCfg[i].u8_Enable)
		{
			rcu_periph_clock_enable(sg_UartCfg[i].u32_TX_RCU_GPIOx);
			rcu_periph_clock_enable(sg_UartCfg[i].u32_RX_RCU_GPIOx);
			rcu_periph_clock_enable(sg_UartCfg[i].u32_UARTx_RCU);
			rcu_periph_clock_enable(RCU_AF);

			gpio_init(sg_UartCfg[i].u32_TX_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_TX_GPIO_Pin);
			gpio_init(sg_UartCfg[i].u32_RX_GPIOx, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_RX_GPIO_Pin);
			
			if(sg_UartCfg[i].u32_UARTx_PERIPH == USART5)
			{
				/* configure USART5 Tx Rx as alternate function */
				if((sg_UartCfg[i].u32_TX_GPIOx == GPIOC) && (sg_UartCfg[i].u16_TX_GPIO_Pin == GPIO_PIN_6))
				{
					gpio_afio_port_config(AFIO_PC6_USART5_CFG, ENABLE);
				}
				else if((sg_UartCfg[i].u32_TX_GPIOx == GPIOA) && (sg_UartCfg[i].u16_TX_GPIO_Pin == GPIO_PIN_11))
				{
					gpio_afio_port_config(AFIO_PA11_USART5_CFG, ENABLE);
				}
				if((sg_UartCfg[i].u32_RX_GPIOx == GPIOC) && (sg_UartCfg[i].u16_RX_GPIO_Pin == GPIO_PIN_7))
				{
					gpio_afio_port_config(AFIO_PC7_USART5_CFG, ENABLE);
				}
				else if((sg_UartCfg[i].u32_RX_GPIOx == GPIOA) && (sg_UartCfg[i].u16_RX_GPIO_Pin == GPIO_PIN_12))
				{
					gpio_afio_port_config(AFIO_PA12_USART5_CFG, ENABLE);
				}
			}

			usart_deinit(sg_UartCfg[i].u32_UARTx_PERIPH);
			usart_baudrate_set(sg_UartCfg[i].u32_UARTx_PERIPH, sg_UartCfg[i].u32_BaudRate);
			usart_word_length_set(sg_UartCfg[i].u32_UARTx_PERIPH, USART_WL_8BIT);
			usart_stop_bit_set(sg_UartCfg[i].u32_UARTx_PERIPH, USART_STB_1BIT);
			usart_parity_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_PM_NONE);
			usart_receive_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_RECEIVE_ENABLE);
			usart_transmit_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_TRANSMIT_ENABLE);

			nvic_irq_enable(sg_UartCfg[i].u32_UARTx_IRQ, 0, 1);
			
			if(sg_UartCfg[i].u32_UARTx_PERIPH == USART5)
			{
				usart5_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART5_FLAG_IDLE);
				usart5_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART5_FLAG_RBNE);
				usart5_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART5_INT_RBNE);
				usart5_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART5_INT_IDLE);
			}
			else
			{
				usart_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART_FLAG_IDLE);
				usart_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART_FLAG_RBNE);
				usart_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART_INT_RBNE);
				usart_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART_INT_IDLE);
			}

			usart_enable(sg_UartCfg[i].u32_UARTx_PERIPH);
			if(sg_UartCfg[i].u8_ManageChannel < sizeof(sg_AppUartManage)/ sizeof(STRU_UART_MANAGE))
			{
				sg_AppUartManage[sg_UartCfg[i].u8_ManageChannel].u8_UartInited = 1;
			}
		}
	}
}
void BspUartInitGprsIpd(void)
{
	uint8_t i = E_UART2_INDEX;
    rcu_periph_clock_enable(sg_UartCfg[i].u32_TX_RCU_GPIOx);
    rcu_periph_clock_enable(sg_UartCfg[i].u32_RX_RCU_GPIOx);

    gpio_init(sg_UartCfg[i].u32_TX_GPIOx, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_TX_GPIO_Pin);
    gpio_init(sg_UartCfg[i].u32_RX_GPIOx, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_RX_GPIO_Pin);
}

void BspUartInitGprs(void)
{
	uint8_t i = E_UART2_INDEX;
    rcu_periph_clock_enable(sg_UartCfg[i].u32_TX_RCU_GPIOx);
    rcu_periph_clock_enable(sg_UartCfg[i].u32_RX_RCU_GPIOx);
    rcu_periph_clock_enable(sg_UartCfg[i].u32_UARTx_RCU);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(sg_UartCfg[i].u32_TX_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_TX_GPIO_Pin);
    gpio_init(sg_UartCfg[i].u32_RX_GPIOx, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, sg_UartCfg[i].u16_RX_GPIO_Pin);

    usart_deinit(sg_UartCfg[i].u32_UARTx_PERIPH);
    usart_baudrate_set(sg_UartCfg[i].u32_UARTx_PERIPH, sg_UartCfg[i].u32_BaudRate);
    usart_word_length_set(sg_UartCfg[i].u32_UARTx_PERIPH, USART_WL_8BIT);
    usart_stop_bit_set(sg_UartCfg[i].u32_UARTx_PERIPH, USART_STB_1BIT);
    usart_parity_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_PM_NONE);
    usart_receive_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_RECEIVE_ENABLE);
    usart_transmit_config(sg_UartCfg[i].u32_UARTx_PERIPH, USART_TRANSMIT_ENABLE);

    nvic_irq_enable(sg_UartCfg[i].u32_UARTx_IRQ, 0, 1);
    
    usart_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART_FLAG_IDLE);
    usart_flag_clear(sg_UartCfg[i].u32_UARTx_PERIPH, USART_FLAG_RBNE);
    usart_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART_INT_RBNE);
    usart_interrupt_enable(sg_UartCfg[i].u32_UARTx_PERIPH, USART_INT_IDLE);

    usart_enable(sg_UartCfg[i].u32_UARTx_PERIPH);
    if(sg_UartCfg[i].u8_ManageChannel < sizeof(sg_AppUartManage)/ sizeof(STRU_UART_MANAGE))
    {
        sg_AppUartManage[sg_UartCfg[i].u8_ManageChannel].u8_UartInited = 1;
    }
}



static STRU_UART_MANAGE* Bsp_ComToUart(E_UART_CHANNEL_LIST eUartId)
{
	if(eUartId < E_MAX_UART_NUMBER)
	{
		if(sg_UartCfg[eUartId].u8_ManageChannel < sizeof(sg_AppUartManage)/ sizeof(STRU_UART_MANAGE)) 
		{
			return &sg_AppUartManage[sg_UartCfg[eUartId].u8_ManageChannel];
		}
	}
	else
	{
		;
	}
	return NULL;
}

/*从串口接收中断中循环保存数据到FIFO中*/
static void Bsp_SaveByte(STRU_UART_MANAGE *pstu_UartManage, uint8_t u8_Byte)
{
    uint16_t uIdx = 0;

    /*缓存长度不够丢掉*/
    if (pstu_UartManage->u16_RxLen >= pstu_UartManage->u16_RxBufSize) {
        return;
    }
    if (pstu_UartManage->u16_RxTail >= pstu_UartManage->u16_RxBufSize) {
        pstu_UartManage->u16_RxTail = 0;
    }
    pstu_UartManage->pu8_RxBuf[pstu_UartManage->u16_RxTail++] = u8_Byte;
    pstu_UartManage->u16_RxLen++;
}

/*从串口接收中断中循环保存数据到FIFO中*/
static void Bsp_SaveBytes(STRU_UART_MANAGE *pstu_UartManage, uint8_t *u8_Bytes, uint16_t len)
{
    uint16_t fontLen = 0;
    uint16_t remainLen = len;
    uint16_t uIdx = pstu_UartManage->u16_RxTail + len;

    if (uIdx >= pstu_UartManage->u16_RxBufSize) {
        fontLen = pstu_UartManage->u16_RxBufSize - pstu_UartManage->u16_RxTail;
        remainLen = len - fontLen;
        memcpy(&pstu_UartManage->pu8_RxBuf[pstu_UartManage->u16_RxTail], u8_Bytes, fontLen);
        pstu_UartManage->u16_RxTail = 0;
    }
    memcpy(&pstu_UartManage->pu8_RxBuf[pstu_UartManage->u16_RxTail], &u8_Bytes[fontLen], remainLen);
    pstu_UartManage->u16_RxTail += remainLen;
    pstu_UartManage->u16_RxLen += len;
}
// 取串口发送完成标志状态
E_UART_ERR Bsp_GetTCSta(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_TCFlag)
{
    STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(e_UartId);
    
    if(NULL == pstu_UartManage)
    {
        return E_UART_ID_ERR;
    }
    
    if(!pstu_UartManage->u8_UartInited)
    {/*串口未初始化*/
        return E_UART_NO_INIT_ERR;
    }
    
    *pu8_TCFlag = pstu_UartManage->u8_BufTC;
	
    return E_UART_NO_ERR;
}

uint16_t fgu16_UartRecvRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_Buf, uint16_t u16_RcvLen, E_UART_ERR *pe_UartErr)
{
    uint16_t u16_RcvCnt = 0;
    *pe_UartErr = E_UART_NO_ERR;
    
    STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(e_UartId);
    
    if(NULL == pstu_UartManage)
    {
        *pe_UartErr = E_UART_ID_ERR;
        return u16_RcvCnt;
    }

    if(!pstu_UartManage->u8_UartInited)
    {/*串口未初始化*/
        *pe_UartErr = E_UART_NO_INIT_ERR;
        return u16_RcvCnt;
    }
    
    while(u16_RcvLen) 
    {
        if(0 == pstu_UartManage->u16_RxLen) 
        {/*接收缓存已无数据*/ 
            break;
        }
        __set_PRIMASK(1);
        pu8_Buf[u16_RcvCnt++] = pstu_UartManage->pu8_RxBuf[pstu_UartManage->u16_RxHead++];
        pstu_UartManage->u16_RxLen--;
        u16_RcvLen--;
       __set_PRIMASK(0);

        if(pstu_UartManage->u16_RxHead >= pstu_UartManage->u16_RxBufSize)
        {/*新的头超出接收缓存长度，从0开始取*/
            pstu_UartManage->u16_RxHead = 0;
        }
    }
    
    return u16_RcvCnt;
}

E_UART_ERR fge_UartSendRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t *pu8_Buf, uint16_t u16_SndLen)
{
    STRU_UART_MANAGE *pstu_UartManage = NULL;
	uint32_t UARTx = 0;
    pstu_UartManage = Bsp_ComToUart(e_UartId);
    
    if(NULL == pstu_UartManage)
    {
        return E_UART_ID_ERR;
    }
    
    if(!pstu_UartManage->u8_UartInited)
    {
        return E_UART_NO_INIT_ERR;
    }

	UARTx = sg32_UartIDtoUARTxPERIH[e_UartId];

    if(pstu_UartManage->u8_UartTxMode == E_TX_MODE_DIR)
    {
        while(u16_SndLen != 0)
        {
			/*TODO TIMEOUT ADMIN*/
			if(UARTx == USART5)
			{
				if (usart5_flag_get(UARTx, USART5_FLAG_TBE))
				{
					usart_data_transmit(UARTx, *(pu8_Buf++));
					u16_SndLen--;
				}
			}
			else
			{
				if (usart_flag_get(UARTx, USART_FLAG_TBE))
				{
					usart_data_transmit(UARTx, *(pu8_Buf++));
					u16_SndLen--;
				}
			}
        }
    }
    else if(pstu_UartManage->u8_UartTxMode == E_TX_MODE_INT)
    {
        uint16_t u16_Idx = 0;
        pstu_UartManage->u8_BufTC = 0;   // 清发送完成标志
        if(u16_SndLen > pstu_UartManage->u16_TxBufSize) // 发送长度超缓存
        {
            return E_UART_BUF_LEN_ERR;
        }
        
        for(uint16_t i = 0; i < u16_SndLen; i++)
        {
            if(pstu_UartManage->u16_TxLen < pstu_UartManage->u16_TxBufSize) // 未存满
            {
                /*关中断*/ 
                u16_Idx = (pstu_UartManage->u16_TxRead + pstu_UartManage->u16_TxLen++) % pstu_UartManage->u16_TxBufSize;
                pstu_UartManage->pu8_TxBuf[u16_Idx] = pu8_Buf[i];
                /*开中断*/
            }
            else
            {
                return E_UART_BUF_LEN_ERR;
            }
        }
		if(UARTx == USART5)
		{
			usart5_interrupt_enable(UARTx, USART5_INT_TBE);
		}
		else
		{
			usart_interrupt_enable(UARTx, USART_INT_TBE);
		}
		
    }
	else if(pstu_UartManage->u8_UartTxMode == E_TX_MODE_DMA)
    {
        pstu_UartManage->pu8_TxBuf = pu8_Buf;
        // DMA_SetCurrDataCounter(pstuUart->pDMAy_Streamx, u16_SndLen);
        // DMA_Cmd(pstuUart->pDMAy_Streamx, ENABLE);
    }
	else
	{
		;
	}
   
    return E_UART_NO_ERR;
}

// 清串口FIFO
E_UART_ERR fge_UartClearRxBufRoute(E_UART_CHANNEL_LIST e_UartId)
{
    STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(e_UartId);
    
    if(NULL == pstu_UartManage)
    {
        return E_UART_ID_ERR;
    }
    
    if(!pstu_UartManage->u8_UartInited)
    {
        return E_UART_NO_INIT_ERR;
    }
	memset(pstu_UartManage->pu8_RxBuf, 0, pstu_UartManage->u16_RxBufSize);
	
    __set_PRIMASK(1);
    pstu_UartManage->u16_RxHead = 0;
    pstu_UartManage->u16_RxLen = 0;
    pstu_UartManage->u16_RxTail = 0;
    __set_PRIMASK(0);

    return E_UART_NO_ERR;
}

void fgv_setRecvCompleteFlagRoute(E_UART_CHANNEL_LIST e_UartId, uint8_t flag)
{
	sgu8_RxCompFlag[e_UartId] = flag;
}

uint8_t fgu8_getRecvCompleteFlagRoute(E_UART_CHANNEL_LIST e_UartId)
{
	return sgu8_RxCompFlag[e_UartId];
}

uint32_t fgu32_AppUartInit(STRU_UART_MANAGE* pstu_UartManage)
{
     BspUartInit();

     CCU_UartRxDmaConfig();
     CCU_UartTxDmaConfig();

    //  GPRS_UartTxDmaConfig();
    //  GPRS_UartRxDmaConfig();

     return E_UART_NO_ERR;
}

void USART_IRQHandler(uint32_t usart_periph, E_UART_CHANNEL_LIST uartList)
{
	uint32_t clear = clear;
	STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(uartList);
	
	if(NULL == pstu_UartManage)
	{
		return;
	}

    /* 接收中断处理 */
	if(usart_interrupt_flag_get(usart_periph, USART_INT_FLAG_RBNE))
	{
		Bsp_SaveByte((STRU_UART_MANAGE *)pstu_UartManage, (uint8_t)usart_data_receive(usart_periph));
		usart_interrupt_flag_clear(usart_periph, USART_INT_FLAG_RBNE);
	}
	if(usart_flag_get(usart_periph, USART_FLAG_ORERR))
	{
		clear = USART_STAT0(usart_periph);
		usart_data_receive(usart_periph);
	}
	if(usart_flag_get(usart_periph, USART_FLAG_NERR))
	{
		clear = USART_STAT0(usart_periph);
		usart_data_receive(usart_periph);
	}
	if(usart_flag_get(usart_periph, USART_FLAG_FERR))
	{
		clear = USART_STAT0(usart_periph);
		usart_data_receive(usart_periph);
	}
	if(usart_flag_get(usart_periph, USART_FLAG_PERR))
	{
		clear = USART_STAT0(usart_periph);
		usart_data_receive(usart_periph);
	}
	else
	{
		;
	}

	if (RESET != usart_interrupt_flag_get(usart_periph, USART_INT_FLAG_IDLE))
	{
		clear = USART_STAT0(usart_periph);
		clear = (uint16_t)(GET_BITS(USART_DATA(usart_periph), 0U, 8U));
		fgv_setRecvCompleteFlagRoute(uartList, 1);
	}

    /* 发送中断处理 */
	if(usart_interrupt_flag_get(usart_periph, USART_INT_FLAG_TBE))
	{
        if(pstu_UartManage->u16_TxLen == 0) 
        {/* FIFO中数据发送完 */
			usart_interrupt_disable(usart_periph, USART_INT_TBE);
            usart_interrupt_enable(usart_periph, USART_INT_TC);
        }
        else
        {
            usart_data_transmit(usart_periph , pstu_UartManage->pu8_TxBuf[pstu_UartManage->u16_TxHead++]);
            pstu_UartManage->u16_TxHead %= pstu_UartManage->u16_TxBufSize;
            pstu_UartManage->u16_TxLen--; 
        }
	}
	else if(usart_interrupt_flag_get(usart_periph, USART_INT_FLAG_TC))
	{
	    if(pstu_UartManage->u16_TxLen == 0)
        {/* 关TC中断 */
			usart_interrupt_disable(usart_periph, USART_INT_TC);
            pstu_UartManage->u8_BufTC = 1; // 一帧发送完成
        }
        else
        {
            usart_data_transmit(usart_periph, pstu_UartManage->pu8_TxBuf[pstu_UartManage->u16_TxHead++]);
            pstu_UartManage->u16_TxHead %= pstu_UartManage->u16_TxBufSize;
            pstu_UartManage->u16_TxLen--;
        }
	}
}

void USART0_IRQHandler(void)
{
    USART_IRQHandler(USART0, E_UART0_INDEX);
}

void USART1_IRQHandler(void)
{
	STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(E_UART1_INDEX);
	if(NULL == pstu_UartManage)
	{
		return;
	}
    
    if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE))
    {
        uint8_t recvLen = pstu_UartManage->u16_RxBufSize - dma_transfer_number_get(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);

        Bsp_SaveBytes((STRU_UART_MANAGE *)pstu_UartManage, s_uartCcuRx_dmaBuf, recvLen);

        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_IDLE);
        
        usart_data_receive(USART1);
        
        dma_channel_disable(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);

        dma_memory_address_config(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL, (uint32_t)s_uartCcuRx_dmaBuf);
        dma_transfer_number_config(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL, CCU_UART_DMAx_RX_LEN);
        
        dma_channel_enable(CCU_UART_DMAx, CCU_UART_DMAx_RX_CHANNEL);
    }

}

void USART2_IRQHandler(void)
{
    USART_IRQHandler(USART2, E_UART2_INDEX);
    return;
    STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(E_UART2_INDEX);
	if(NULL == pstu_UartManage)
	{
		return;
	}
    
    if(usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE))
    {
        uint8_t recvLen = pstu_UartManage->u16_RxBufSize - dma_transfer_number_get(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);

        Bsp_SaveBytes((STRU_UART_MANAGE *)pstu_UartManage, s_uartGPRSRx_dmaBuf, recvLen);

        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_IDLE);
        
        usart_data_receive(USART2);
        
        dma_channel_disable(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);

        dma_memory_address_config(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL, (uint32_t)s_uartGPRSRx_dmaBuf);
        dma_transfer_number_config(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL, GPRS_UART_DMAx_RX_LEN);
        memset(s_uartGPRSRx_dmaBuf, 0, GPRS_UART_DMAx_RX_LEN);
        
        dma_channel_enable(GPRS_UART_DMAx, GPRS_UART_DMAx_RX_CHANNEL);
    }

}

void UART3_IRQHandler(void)
{
    USART_IRQHandler(UART3, E_UART3_INDEX);
}

void UART4_IRQHandler(void)
{
    USART_IRQHandler(UART4, E_UART4_INDEX);
}

void USART5_IRQHandler(void)
{
	uint32_t clear = clear;
	uint32_t usart_periph = USART5;
	STRU_UART_MANAGE *pstu_UartManage = NULL;
    pstu_UartManage = Bsp_ComToUart(E_UART5_INDEX);
	
	if(NULL == pstu_UartManage)
	{
		return;
	}

    /* 接收中断处理 */
	if((RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_RBNE)) && (RESET != usart5_flag_get(USART5, USART5_FLAG_RBNE)))
	{
		Bsp_SaveByte((STRU_UART_MANAGE *)pstu_UartManage, (uint8_t)usart_data_receive(USART5));
		usart5_interrupt_flag_clear(USART5, USART5_INT_FLAG_RBNE);
	}
	else if(usart5_flag_get(USART5, USART5_FLAG_ORERR))
	{
		usart_data_receive(USART5);
	}
	else if(usart5_flag_get(USART5, USART5_FLAG_NERR))
	{
		usart_data_receive(USART5);
	}
	else if(usart5_flag_get(USART5, USART5_FLAG_FERR))
	{
		usart_data_receive(USART5);
	}
	else if(usart5_flag_get(USART5, USART5_FLAG_PERR))
	{
		usart_data_receive(USART5);
	}
	else
	{
		;
	}

	if ((RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_IDLE)) && (RESET != usart5_flag_get(USART5, USART5_FLAG_IDLE)))
	{
		// clear = USART5_STAT(USART5);
		// clear = (uint16_t)(GET_BITS(USART5_RDATA(USART5), 0U, 8U));
		usart5_interrupt_flag_clear(USART5, USART5_INT_FLAG_IDLE);
		fgv_setRecvCompleteFlagRoute(E_UART5_INDEX, 1);
	}

    /* 发送中断处理 */
	if((RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_TBE)) && (RESET != usart5_flag_get(USART5, USART5_FLAG_TBE)))
	{
        if(pstu_UartManage->u16_TxLen == 0) 
        {/* FIFO中数据发送完 */
			usart5_interrupt_disable(USART5, USART5_INT_TBE);
            usart5_interrupt_enable(USART5, USART5_INT_TC);
        }
        else
        {
            usart_data_transmit(USART5 , pstu_UartManage->pu8_TxBuf[pstu_UartManage->u16_TxHead++]);
			pstu_UartManage->u16_TxHead++;
            pstu_UartManage->u16_TxHead %= pstu_UartManage->u16_TxBufSize;
            pstu_UartManage->u16_TxLen--; 
        }
	}
	else if((RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_TC)))
	{
		usart5_interrupt_flag_clear(USART5, USART5_INT_FLAG_TC);
	    if(pstu_UartManage->u16_TxLen == 0)
        {/* 关TC中断 */
			usart5_interrupt_disable(USART5, USART5_INT_TC);
            pstu_UartManage->u8_BufTC = 1; // 一帧发送完成
        }
        else
        {
            usart_data_transmit(USART5, pstu_UartManage->pu8_TxBuf[pstu_UartManage->u16_TxHead++]);
            pstu_UartManage->u16_TxHead %= pstu_UartManage->u16_TxBufSize;
            pstu_UartManage->u16_TxLen--;
        }
	}
}

FILE __stdout;
__ASM (".global __use_no_semihosting");
int _ttywrch(int ch)
{
	ch = ch;
	return 0;
}
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x) 
{ 
	x = x; 
}
int fputc(int ch, FILE *f)
{
	usart_data_transmit(USART0, (uint8_t)ch);
	while (RESET == usart_flag_get(USART0, USART_FLAG_TC)) {
		;
	}
	// usart_data_transmit(USART5, (uint8_t)ch);
	// while (RESET == usart5_flag_get(USART5, USART5_FLAG_TBE)) {
	// 	;
	// }

	return ch;
}
