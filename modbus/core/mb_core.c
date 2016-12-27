#include "mb_core.h"
#include "mb_config.h"
#include "mb_crc.h"
#include "mb_port.h"
#include "mb_app.h"
#include "stm32f10x.h"
#include "usart.h"
#include "timer.h"
#include "stdio.h"
#include "stdlib.h"

static MB_EventType_t eQueuedEvent;
static BOOL     MB_EventMutex;		//事件互斥锁，即一个信号发送后，如果还没有得到处理，那么不允许再次发送信号

unsigned char MB_ADD=0x01;					//从机地址
unsigned short REG_HOLDING_START=20;			//保持寄存器起始地址
unsigned short REG_HOLDING_NREGS=5;			//保持寄存器个数
unsigned short REG_INPUT_START=30;
unsigned short REG_INPUT_NREGS=5;				//寄存器采用动态分配

unsigned char M_SendBuff[M_SendBuffLen];
unsigned short M_SendCnt=0;
unsigned short M_SendLen=0;

unsigned char M_ReceiveBuff[M_ReceiveBuffLen];
unsigned short M_ReceiveCnt=0;

unsigned char S_SendBuff[S_SendBuffLen];
unsigned short S_SendCnt=0;
unsigned short S_SendLen=0;

unsigned char S_ReceiveBuff[S_ReceiveBuffLen];
unsigned short S_ReceiveCnt=0;

unsigned short *usRegHoldingBuf;
unsigned short *usRegInputBuf;

mb_pro_t mb_pro_m;
mb_pro_t mb_pro_s;
unsigned long usTimerT35_50us;

BOOL MB_EventInit( void )
{
    MB_EventMutex = FALSE;		//事件锁
    return TRUE;
}

BOOL MB_EventPost( MB_EventType_t eEvent )
{
    MB_EventMutex = TRUE;
    eQueuedEvent = eEvent;
    return TRUE;
}

BOOL MB_EventGet( MB_EventType_t * eEvent )
{
    BOOL            xEventHappened = FALSE;

    if( MB_EventMutex )
    {
        *eEvent = eQueuedEvent;
        MB_EventMutex = FALSE;
        xEventHappened = TRUE;
    }
    return xEventHappened;
}

MB_EventType_t MB_EventState()
{
	return eQueuedEvent;
}

void MB_RegInit()
{
	usRegHoldingBuf=malloc(REG_HOLDING_NREGS*sizeof(unsigned short));
	usRegInputBuf=malloc(REG_INPUT_NREGS*sizeof(unsigned short));
}

void MB_Init(uint32_t ulBaudRate)
{
    /* Modbus RTU uses 8 Databits. */
    USART_Configuration(UART5, ulBaudRate);

	/* If baudrate > 19200 then we should use the fixed timer values
	 * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
	 */
	if( ulBaudRate > 19200 )
	{
		usTimerT35_50us = 35;       /* 1800us. */
	}
	else
	{
		/* The timer reload value for a character is given by:
		 *
		 * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
		 *             = 11 * Ticks_per_1s / Baudrate
		 *             = 220000 / Baudrate
		 * The reload for t3.5 is 1.5 times this value and similary
		 * for t3.5.
		 */
		usTimerT35_50us = ( 7UL * 220000UL ) / ( 2UL * ulBaudRate );
	}
	TIM3_Int_Init(usTimerT35_50us,3600);
}

void MB_Poll()
{
	MB_EventType_t eEvent;

	if( MB_EventGet( &eEvent ) == TRUE )
	{
		switch(eEvent)
		{
		case EV_IDLE:
			break;
		case EV_M_SEND:						//主机状态部分
//			mb_ms_SendData(&gb_mb_pro);
//			MB_EventPost(EV_M_WAIT);
			USART_IRQSet(1,0);
			break;
		case EV_M_WAIT:						//等待接收，同时校验等待超时
			break;
		case EV_M_RECEIVEING:				//正在接收，同时校验完成超时
			break;
		case EV_M_RECEIVED:					//接收完成
			//此处对接收的数据进行处理
			M_ReceiveCnt=0;					//该值表示接收数据的长度，当数据处理完成后记得清零，因为我没有对接收做处理，所以直接清零
			MB_EventPost(EV_IDLE);		//接收处理完成，一次协议通讯结束，切换到idle状态
			break;
		case EV_S_RECEIVEING:				//正在接收，同时校验完成超时
			break;
		case EV_S_RECEIVED:					//数据接收完成
			//对数据进行处理
			MB_S_Execute(S_ReceiveBuff,S_ReceiveCnt);
			S_ReceiveCnt=0;
			MB_EventPost(EV_S_SEND);	//处理完成，切换到发送状态发送响应
			break;
		case EV_S_SEND:
			USART_IRQSet(1,0);				//开始发送，发送完成后会自动切换到空闲状态
			break;
		default:
			break;
		}
	}
}

void MB_M_FormatData()
{
	unsigned short crcval=0;
	unsigned char *databuff=M_SendBuff+MB_DATA_INDEX;
	M_SendBuff[MB_ADDR_INDEX]=MB_ADD;
	M_SendBuff[MB_FUNCID_INDEX]=mb_pro_m.funcid;

	switch(mb_pro_m.funcid)
	{
	case FUNCID_READHDREG:
	{
		databuff[0]=((unsigned short)mb_pro_m.startadd)>>8;
		databuff[1]=(unsigned short)mb_pro_m.startadd;
		databuff[2]=((unsigned short)mb_pro_m.regN)>>8;
		databuff[3]=(unsigned short)mb_pro_m.regN;

		crcval=usMBCRC16((unsigned char *)M_SendBuff,6);
		databuff[4]=( unsigned char )( crcval & 0xFF );
		databuff[5]=( unsigned char )( crcval >> 8 );
		M_SendLen=8;
		M_SendCnt=0;
		break;
	}
	case FUNCID_READIPREG:
	{
		databuff[0]=((unsigned short)mb_pro_m.startadd)>>8;
		databuff[1]=(unsigned short)mb_pro_m.startadd;
		databuff[2]=((unsigned short)mb_pro_m.regN)>>8;
		databuff[3]=(unsigned short)mb_pro_m.regN;

		crcval=usMBCRC16((unsigned char *)M_SendBuff,6);
		databuff[4]=( unsigned char )( crcval & 0xFF );
		databuff[5]=( unsigned char )( crcval >> 8 );
		M_SendLen=8;
		M_SendCnt=0;
		break;
	}
	case FUNCID_WROHDREG:
	{
		databuff[0]=((unsigned short)mb_pro_m.startadd)>>8;
		databuff[1]=(unsigned short)mb_pro_m.startadd;
		databuff[2]=mb_pro_m.databuff[0];
		databuff[3]=mb_pro_m.databuff[1];

		crcval=usMBCRC16((unsigned char *)M_SendBuff,6);
		databuff[4]=( unsigned char )( crcval & 0xFF );
		databuff[5]=( unsigned char )( crcval >> 8 );
		M_SendLen=8;
		M_SendCnt=0;
		break;
	}
	case FUNCID_WRNHDREG:
	{
		unsigned char i=0;
		databuff[0]=((unsigned short)mb_pro_m.startadd)>>8;
		databuff[1]=(unsigned short)mb_pro_m.startadd;

		databuff[2]=((unsigned short)mb_pro_m.regN)>>8;
		databuff[3]=(unsigned short)mb_pro_m.regN;

		databuff[4]=(unsigned short)mb_pro_m.regN*2;
		for(i=0;i<mb_pro_m.regN;i++)
		{
			databuff[5+i*2]=mb_pro_m.databuff[i*2+1];		//注意字节顺序
			databuff[6+i*2]=mb_pro_m.databuff[i*2];
		}

		crcval=usMBCRC16((uint8_t *)M_SendBuff,7+mb_pro_m.regN*2);
		databuff[5+mb_pro_m.regN*2]=( uint8_t )( crcval & 0xFF );
		databuff[6+mb_pro_m.regN*2]=( uint8_t )( crcval >> 8 );
		M_SendLen=9+mb_pro_m.regN*2;
		M_SendCnt=0;
		break;
	}
	default:
		break;
	}


}

void MB_S_Execute(uint8_t *buff,uint16_t len)
{
	uint16_t crcval=0;
	uint8_t mbId=buff[0];
	uint8_t func=buff[1];

	uint16_t add=((unsigned short)buff[2]<<8)+buff[3];

	S_SendBuff[0]=MB_ADD;
	S_SendBuff[1]=func;

	switch(func)
	{
	case FUNCID_READHDREG:		//读保持寄存器
	{
		unsigned short num=((unsigned short)buff[4]<<8)+buff[5];
		if(add<REG_HOLDING_START||(add+num)>(REG_HOLDING_START+REG_HOLDING_NREGS))
		{
			//发送错误码
		}
		else
		{
			//正常响应数据
			mb_pro_s.funcid=func;
			mb_pro_s.startadd=add;
			mb_pro_s.regN=num;
			mb_pro_s.mode=1;
			mb_pro_s.databuff=S_SendBuff+3;
			S_SendBuff[2]=2*num;
			eMBRegHoldingCB(&mb_pro_s);		//此函数会填充数据段
			crcval=usMBCRC16(S_SendBuff,3+2*num);
			S_SendBuff[3+2*num]=( unsigned char )( crcval & 0xFF );
			S_SendBuff[4+2*num]=( unsigned char )( crcval >> 8 );
			S_SendLen=5+2*num;
			S_SendCnt=0;
		}
		break;
	}
	case FUNCID_READIPREG:		//读输入寄存器
	{
		unsigned short num=((unsigned short)buff[4]<<8)+buff[5];
		if(add<REG_INPUT_START||(add+num)>(REG_INPUT_START+REG_INPUT_NREGS))
		{
			//发送错误码
		}
		else
		{
			//正常响应数据
			mb_pro_s.funcid=func;
			mb_pro_s.startadd=add;
			mb_pro_s.regN=num;
			mb_pro_s.mode=1;
			mb_pro_s.databuff=S_SendBuff+3;
			S_SendBuff[2]=2*num;
			eMBRegInputCB(&mb_pro_s);		//此函数会填充数据段
			crcval=usMBCRC16(S_SendBuff,3+2*num);
			S_SendBuff[3+2*num]=( unsigned char )( crcval & 0xFF );
			S_SendBuff[4+2*num]=( unsigned char )( crcval >> 8 );
			S_SendLen=5+2*num;
			S_SendCnt=0;
		}
		break;
	}
	case FUNCID_WROHDREG:		//写单个保持寄存器
	{
		if(add<REG_HOLDING_START||add>(REG_HOLDING_START+REG_HOLDING_NREGS-1))
		{
			//发送错误码
		}
		else
		{
			//正常响应数据
			mb_pro_s.funcid=func;
			mb_pro_s.startadd=add;
			mb_pro_s.regN=1;
			mb_pro_s.mode=2;
			mb_pro_s.databuff=buff+4;
			S_SendBuff[2]=buff[2];
			S_SendBuff[3]=buff[3];
			S_SendBuff[4]=buff[4];
			S_SendBuff[5]=buff[5];
			eMBRegHoldingCB(&mb_pro_s);
			crcval=usMBCRC16(S_SendBuff,6);
			S_SendBuff[6]=( unsigned char )( crcval & 0xFF );
			S_SendBuff[7]=( unsigned char )( crcval >> 8 );
			S_SendLen=8;
			S_SendCnt=0;
		}
		break;
	}
	case FUNCID_WRNHDREG:		//写多个保持寄存器
	{
		unsigned short num=((unsigned short)buff[4]<<8)+buff[5];
		if(add<REG_HOLDING_START||add+num>(REG_HOLDING_START+REG_HOLDING_NREGS))
		{
			//发送错误码
		}
		else
		{
			//正常响应数据
			mb_pro_s.funcid=func;
			mb_pro_s.startadd=add;
			mb_pro_s.regN=num;
			mb_pro_s.mode=2;
			S_SendBuff[2]=buff[2];
			S_SendBuff[3]=buff[3];
			S_SendBuff[4]=buff[4];
			S_SendBuff[5]=buff[5];
			mb_pro_s.databuff=buff+7;
			eMBRegHoldingCB(&mb_pro_s);
			crcval=usMBCRC16(S_SendBuff,6);
			S_SendBuff[6]=( unsigned char )( crcval & 0xFF );
			S_SendBuff[7]=( unsigned char )( crcval >> 8 );
			S_SendLen=8;
			S_SendCnt=0;
		}
		break;
	}
	default:
		break;
	}
}
