#include "mb_app.h"
#include "mb_config.h"
#include "mb_core.h"
#include "mb_type.h"



void MB_M_app_demo(unsigned char *flag)
{
	MB_EventType_t sta=MB_EventState();

	switch(*flag)
	{
	case 0:
		break;
	case 1:								//读保持寄存器
		if(sta!=EV_IDLE) break;

		mb_pro_m.funcid=FUNCID_READHDREG;
		mb_pro_m.startadd=REG_HOLDING_START;
		mb_pro_m.regN=2;
		MB_M_FormatData();
		MB_EventPost(EV_M_SEND);	//开始发送
		*flag=0;
		break;
	case 2:								//读输入寄存器
		if(sta!=EV_IDLE) break;

		mb_pro_m.funcid=FUNCID_READIPREG;
		mb_pro_m.startadd=REG_INPUT_START;
		mb_pro_m.regN=2;
		MB_M_FormatData();
		MB_EventPost(EV_M_SEND);	//开始发送
		*flag=0;
		break;
	case 3:								//写单个寄存器
	{
		unsigned short regn=2;						//写两个寄存器，即四个字节
		unsigned short datas[2]={0x0101,0x0102};

		if(sta!=EV_IDLE) break;

		mb_pro_m.funcid=FUNCID_WROHDREG;
		mb_pro_m.startadd=REG_HOLDING_START;
		mb_pro_m.regN=1;
		mb_pro_m.databuff=(unsigned char *)datas;

		MB_M_FormatData();
		MB_EventPost(EV_M_SEND);	//开始发送
		*flag=0;
		break;
	}
	case 4:								//写多个寄存器
	{
		unsigned short regn=2;						//写两个寄存器，即四个字节
		unsigned short datas[2]={0x0102,0x0304};

		mb_pro_m.funcid=FUNCID_WRNHDREG;
		mb_pro_m.startadd=REG_HOLDING_START;
		mb_pro_m.regN=regn;
		mb_pro_m.databuff=(unsigned char *)datas;
		MB_M_FormatData();
		MB_EventPost(EV_M_SEND);	//开始发送
		*flag=0;
		break;
	}
	default:
		*flag=0;
		break;
	}
}

//输入寄存器
void eMBRegInputCB(mb_pro_t *p_prot)
{
    int             iRegIndex;

    iRegIndex = ( int )( p_prot->startadd - REG_INPUT_START );
	while( p_prot->regN > 0 )
	{
		*p_prot->databuff++ =
			( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
		*p_prot->databuff++ =
			( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
		iRegIndex++;
		p_prot->regN--;
	}
}

//保持寄存器
void eMBRegHoldingCB(mb_pro_t *p_prot)
{
	int iRegIndex;
	iRegIndex = ( int )( p_prot->startadd - REG_HOLDING_START );

	while(p_prot->regN > 0)
	{
		if(p_prot->mode==1)
		{
			*p_prot->databuff++ =
				( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
			*p_prot->databuff++ =
				( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
		}
		else if(p_prot->mode==2)		//高八位在前，低八位在后
		{
			usRegHoldingBuf[iRegIndex]=*p_prot->databuff;
			usRegHoldingBuf[iRegIndex]<<=8;
			p_prot->databuff++;
			usRegHoldingBuf[iRegIndex]|=*p_prot->databuff;
			p_prot->databuff++;
		}

		iRegIndex++;
		p_prot->regN--;
	}
}
