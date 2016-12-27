#ifndef  __USART_H
#define  __USART_H

#include "stm32f10x.h"

#define Modbus_Port USART1

#define RS_485_RX_EN() 	do{ GPIO_ResetBits(GPIOB , GPIO_Pin_14); GPIO_ResetBits(GPIOC , GPIO_Pin_7);}while(0)	//接收使能
#define RS_485_TX_EN() 	do{ GPIO_SetBits(GPIOB , GPIO_Pin_14); GPIO_SetBits(GPIOC , GPIO_Pin_7);}while(0)	//发送使能

void rs485_ctrl_init(void);
void USART_Configuration(USART_TypeDef *USARTx,uint32_t bdrate);
void UART5_SendData(u8 ch);
void UART5_SendNData(u8 *str,u16 len);
void UART5_SendString(u8 *str);                      

#endif
