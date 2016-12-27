#include "usart.h"
#include "string.h"
#include "mb_port.h"

//void prvvUARTTxReadyISR( void );
//void prvvUARTRxISR( void );

#if 0
void USART1_Init(UartParam_t *UartParam)
{
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);

		//初始化PA9和PA10	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);		   

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	        
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}

	{
		USART_InitTypeDef USART_InitStructure; 
		UART_ParamSet(&USART_InitStructure,UartParam);
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //发送和接受使能
		USART_Init(USART1, &USART_InitStructure);                                       //初始化串口2
//		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                                  //接收中断使能
		USART_Cmd(USART1, ENABLE);                                                      //使能时串口2                     
//		USART_ClearITPendingBit(USART1, USART_IT_TC);                                   //清除中断TC位
	}
	
//	{
//		NVIC_InitTypeDef   NVIC_InitStructure;
//		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	         //串口2中断
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//3;    //抢占优先级3
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//1;	         //子优先级3
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	 //IRQ通道使能
//		NVIC_Init(&NVIC_InitStructure);                              //根据指定的参数初始化VIC寄存器
//	}
	
	RS_485_TX_EN;
}
#endif

#if 0
void USART1_SendData(u8 ch)
{
//	RS_485_TX_EN;
	
	USART_SendData(USART1,ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==0);
	
//	RS_485_RX_EN;
}

void USART1_SendNData(u8 *str,u16 len)
{
 	u16 i = 0;
//	RS_485_TX_EN;
	for(i = 0;i<len;i++)
	{			
		USART_SendData(USART1,str[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==0);
	}	
//	RS_485_RX_EN;
}

void USART1_SendString(u8 *str)
{
//	RS_485_TX_EN;
	while(*str)
	{
		USART_SendData(USART1,*str);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==0);
		str++;
	}
//	RS_485_RX_EN;
}
#endif

void rs485_ctrl_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void USART_Configuration(USART_TypeDef *USARTx,uint32_t bdrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	if(USARTx==USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = bdrate;
		USART_InitStructure.USART_WordLength=USART_WordLength_8b;
		USART_InitStructure.USART_StopBits=USART_StopBits_1;
		USART_InitStructure.USART_Parity=USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //发送和接受使能
		USART_Init(USART1, &USART_InitStructure);                                       //初始化串口2
		//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);                                  	//接收中断使能
		USART_Cmd(USART1, ENABLE);                                                      //使能时串口2
		//USART_ClearITPendingBit(UART5, USART_IT_TC);                                   //清除中断TC位

		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	         //串口2中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;    //抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	         //子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	 //IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);                              //根据指定的参数初始化VIC寄存器

		rs485_ctrl_init();
		RS_485_RX_EN();
	}
	else if(USARTx==UART5)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO,ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = bdrate;
		USART_InitStructure.USART_WordLength=USART_WordLength_8b;
		USART_InitStructure.USART_StopBits=USART_StopBits_1;
		USART_InitStructure.USART_Parity=USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //发送和接受使能
		USART_Init(UART5, &USART_InitStructure);                                       //初始化串口2
		USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);                                  //接收中断使能
		USART_Cmd(UART5, ENABLE);                                                      //使能时串口2
		//USART_ClearITPendingBit(UART5, USART_IT_TC);                                   //清除中断TC位

		NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;	         //串口2中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;    //抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	         //子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           	 //IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);                              //根据指定的参数初始化VIC寄存器
	}
}

//UASRT1的中断函数
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	//u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//prvvUARTRxISR();
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	else if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		//prvvUARTTxReadyISR();
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}
}

//UASRT2的中断函数
void UART5_IRQHandler(void)                	//串口1中断服务程序
{
	//u8 Res;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);
		//prvvUARTRxISR();
		Receive_IRQ();
	}
	else if(USART_GetITStatus(UART5, USART_IT_TXE) != RESET)
	{
		USART_ClearITPendingBit(UART5, USART_IT_TC);
		//prvvUARTTxReadyISR();
		Send_IRQ();
	}
}

void UART5_SendNData(u8 *str,u16 len)
{
 	unsigned int i = 0;
	for(i = 0;i < len;i++)
	{			
		USART_SendData(UART5,*str);
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==0);
		str++;
	}	
}

void UART5_SendData(u8 ch)
{
	USART_SendData(UART5,ch);
	while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==0);
}

void UART5_SendString(u8 *str)
{
	while(*str)
	{
		USART_SendData(UART5,*str);
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==0);
		str++;
	}
}

u8 GetString(u8 *buff,u8 len,u16 *timeFlag,u16 timeout)
{
	*timeFlag=timeout*2;
	while (len)
	{
		if(USART_GetFlagStatus(UART5, USART_FLAG_RXNE) == RESET)
		{
			if(*timeFlag==0)	return 0;		//超时了
		}
		else
		{
			*buff = (uint8_t)UART5->DR;
			if(*buff=='\r')	return 1;
			buff++;
			len--;
		}
	}
	return 1;												//接收完成
}

