#ifndef STM32_SPI1

#define STM32_SPI1

#include "stm32f30x.h"

/* #################################
 * #     SPI CONFIGURATION         #
 * #################################*/

// ######### SCK Clock speed ########

//#define SPI1_PRESCALER 281250
//#define SPI1_PRESCALER 562500
//#define SPI1_PRESCALER 1125000
//#define SPI1_PRESCALER 2250000
//#define SPI1_PRESCALER 4500000
//#define SPI1_PRESCALER 9000000
//#define SPI1_PRESCALER 18000000
#define SPI1_PRESCALER 36000000


//Clock polarity
//#define SPI1_SCK_IDLE_0
#define SPI1_SCK_IDLE_1

//#define SPI1_MSB_FIRST
#define SPI1_LSB_FIRST


#define SPI1_DATA_SIZE 8
#if SPI1_DATA_SIZE < 4
#error SPI1_DATA_SIZE must be at least 4 bit
#endif

/* #################################
 * #    END SPI CONFIGURATION      #
 * #################################*/

void spi1_init(void);
uint16_t spi1_write(uint8_t value);
void spi1_initGPIO(void);
#endif
