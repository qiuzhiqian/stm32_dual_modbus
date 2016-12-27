#include "mb_port.h"
#include "mb_config.h"
#include "mb_core.h"

#include "stm32f10x.h"

/*******************************************************串口部分*****************************************/
void SendChar(unsigned char ch)
{
	USART_SendData(UART5,ch);
}

unsigned char GetChar()
{
	return USART_ReceiveData(UART5);
}

void USART_IRQSet(unsigned char txIrq,unsigned char rxIrq)
{
	if(1 == rxIrq)
	{
		//RS_485_RX_EN();
		USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	}
	else
	{
		USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
	}

	if(1 == txIrq)
	{
		//RS_485_TX_EN();
		USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
	}
}

void Send_IRQ()
{
	MB_EventType_t sta=MB_EventState();
	switch(sta)
	{
	case EV_M_SEND:
	{
		if(M_SendLen>0)
		{
			SendChar(*(M_SendBuff+M_SendCnt));
			M_SendCnt++;
			M_SendLen--;
		}
		else		//发送最后一个字节
		{
			USART_IRQSet(0,1);				//关闭发送
			MB_EventPost(EV_M_WAIT);
			//开启定时器进行超时计时
			TIM_SetAutoreload(TIM3,usTimerT35_50us*50);
			TIM_IRQSet(1);
		}
		break;
	}
	case EV_S_SEND:
	{
		if(S_SendLen>0)
		{
			SendChar(*(S_SendBuff+S_SendCnt));
			S_SendCnt++;
			S_SendLen--;
		}
		else
		{
			USART_IRQSet(0,1);				//关闭发送,打开接收
			MB_EventPost(EV_IDLE);
		}

		break;
	}
	}

}

void Receive_IRQ()
{
	uint8_t ch;
	ch=GetChar();

	MB_EventType_t sta=MB_EventState();
	switch(sta)
	{
	case EV_M_WAIT:		//响应没有超时
		M_ReceiveBuff[M_ReceiveCnt++]=ch;
		TIM_SetAutoreload(TIM3,usTimerT35_50us);	//恢复定时器超时
		MB_EventPost(EV_M_RECEIVEING);			//切换状态
		break;
	case EV_M_RECEIVEING:
		M_ReceiveBuff[M_ReceiveCnt++]=ch;
		break;
	case EV_IDLE:		//空闲状态有数据来了,说明是开始了从机状态
		S_ReceiveBuff[S_ReceiveCnt++]=ch;
		MB_EventPost(EV_S_RECEIVEING);			//切换状态
		break;
	case EV_S_RECEIVEING:
		S_ReceiveBuff[S_ReceiveCnt++]=ch;
		break;
	}

	//开启并刷新定时器
	TIM_IRQSet(1);
}

void TIM_IRQSet(unsigned char tmIrq)
{
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_SetCounter(TIM3,0);
	if(tmIrq==1)
	{
		TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	}
	else
	{
		TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
	}
}

void Time_IRQ()		//3.5T时间到达，接收完成，
{
	TIM_IRQSet(0);	//关闭定时器

	MB_EventType_t sta=MB_EventState();
	switch(sta)
	{
	case EV_M_WAIT:							//接收等待超时
		MB_EventPost(EV_IDLE);			//放弃掉此次通讯
		TIM_SetAutoreload(TIM3,usTimerT35_50us);	//恢复定时器超时
		break;
	case EV_M_RECEIVEING:
		MB_EventPost(EV_M_RECEIVED);		//接收完成
		break;
	case EV_S_RECEIVEING:
		USART_IRQSet(0,0);		//接收完成，关闭接收中断
		MB_EventPost(EV_S_RECEIVED);			//切换到接收处理
		break;
	}
}

/*****************************************************定时器部分*******************************************/


