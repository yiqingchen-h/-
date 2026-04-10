#ifndef __UART_H
#define __UART_H		
#include "SYS.h"
#include "stdio.h"  //重定向printf使用
#include "string.h" //重定向printf使用
extern unsigned char Rx_Data[20];

void Uart_0_Init(void);

void U1_Rx(void);
#endif