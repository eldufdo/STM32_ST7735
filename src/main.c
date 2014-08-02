#include <stdio.h>
#include "diag/Trace.h"
#include "stm32f30x.h"
#include "spi1.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

void delay(volatile int i) {
	while(i--);
}

int
main(int argc, char* argv[])
{


	while (1) {

	}
}

#pragma GCC diagnostic pop
