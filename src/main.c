//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "stm32f30x_gpio.h"
#include "delay.h"
#include "st7735.h"
// ----------------------------------------------------------------------------
//
// Standalone STM32F3 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
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

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
