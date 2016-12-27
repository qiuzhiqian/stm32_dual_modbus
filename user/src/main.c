#include "stm32f10x.h"
#include "led.h"
#include "timer.h"
#include "mb_config.h"
#include "mb_app.h"
#include "mb_core.h"
#include "mb_port.h"
#include "usart.h"

uint8_t step=0;

int
main( void )
{
	LED_Init();
	MB_Init(115200);
	MB_RegInit();
	//UART5_SendNData("123\r\n",5);
	MB_EventInit();
	USART_IRQSet(0,1);			//开启接收中断
	LED_GREEN(1);

    for( ;; )
    {
    	MB_Poll();
    	MB_M_app_demo(&step);			//主机发送样例
    }
}

