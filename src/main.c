#include "stm32f30x_gpio.h"
#include "delay.h"
#include "st7735.h"


int main(int argc, char* argv[])
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE,ENABLE);
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Pin = GPIO_Pin_15;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&gpio);
	ST7735_Init();
	ST7735_Orientation(1);
	ST7735_Clear(ST7735_Color565(255,255,255));
	ST7735_drawRoundRect(10,10,100,100,10,ST7735_Color565(255,0,0));
	ST7735_drawCircle(50,50,5,ST7735_Color565(255,0,0));
	ST7735_fillCircle(50,80,15,ST7735_Color565(0,255,0));
	while (1)
    {
    }
}
