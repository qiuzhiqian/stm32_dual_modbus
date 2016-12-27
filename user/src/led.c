#include "led.h" 
		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0 | GPIO_Pin_6;//LED 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //翻转速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推免输出方式
	GPIO_Init(GPIOC, &GPIO_InitStructure);             //初始化PB8.9.14.15
}
