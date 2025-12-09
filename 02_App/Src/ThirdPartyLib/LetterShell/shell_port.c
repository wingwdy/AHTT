/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @Copyright (c) 2019 Unicook
 * 
 */

#include "shell.h"
#include "Mcal_Uart.h"



Shell shell;
char shellBuffer[512];

/**
 * @brief 用户shell写
 * 
 * @param data 数据
 */
void userShellWrite(char data)
{
    McalUart_WriteData(eMcalUartChanel_Debug, (uint8_t *)&data, 1);
}


/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @return char 状态
 */
int8_t userShellRead(char *data)
{
    if (McalUart_ReadData(eMcalUartChanel_Debug, (uint8_t *)data, 1) == eGlobalRet_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(void)
{
    shell.write = userShellWrite;
    shell.read = userShellRead;
    shellInit(&shell, shellBuffer, 512);
}