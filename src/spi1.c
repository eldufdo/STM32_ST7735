#include "stm32f30x.h"
#include "spi1.h"

void spi1_init(void) {
	spi1_initGPIO();
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 |= SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SSI;
	// set baud rate 
#if SPI1_PRESCALER == 281250
	// prescaler = 256 --> 72/256= 281.25 kHz
	SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;
#endif
#if SPI1_PRESCALER == 562500
	// prescaler = 128 --> 72/128= 562.5 kHz
	SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1;
#endif
#if SPI1_PRESCALER == 1125000
	// prescaler = 64 --> 72/64= 1.125 MHz
	SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_0;
#endif
#if SPI1_PRESCALER == 2250000
	// prescaler = 32 --> 72/32= 1.125 MHz
	SPI1->CR1 |= SPI_CR1_BR_2;
#endif
#if SPI1_PRESCALER == 4500000
	// prescaler = 16 --> 72/16 = 4.5 MHz
	SPI1->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0;
#endif

#if SPI1_PRESCALER == 9000000
	// prescaler = 8 --> 72/8 = 9 MHz
	SPI1->CR1 |= SPI_CR1_BR_1;
#endif
#if SPI1_PRESCALER == 18000000
	// prescaler = 4 --> 72/4 = 18 MHz
	SPI1->CR1 |= SPI_CR1_BR_0;
#endif
#if SPI1_PRESCALER == 36000000
	// prescaler = 2 --> 72/2= 36 MHz
	SPI1->CR1 &= ~SPI_CR1_BR_2 & ~SPI_CR1_BR_1 & ~SPI_CR1_BR_0;
#endif
	// set as master
	SPI1->CR1 |= SPI_CR1_MSTR;
#ifdef SPI1_SCK_IDLE_1
	SPI1->CR1 |= SPI_CR1_CPOL;
#endif
	SPI1->CR2 = (SPI1_DATA_SIZE-1)<<8;
#if SPI1_DATASIZE < 9
	SPI1->CR2 |= SPI_CR2_FRXTH;
#endif
#ifdef SPI1_LSB_FIRST
	SPI1->CR1 |= SPI_CR1_LSBFIRST;
#endif
	SPI1->CR1 |= SPI_CR1_CPHA;
	// set data size of SPI to 16 bit
	// activate SPI Interface
	SPI1->CR1 |= SPI_CR1_SPE;
}

void spi1_initGPIO(void) {
//enable clock for port E
	RCC->AHBENR |=  RCC_AHBENR_GPIOEEN;
	RCC->AHBENR |=  RCC_AHBENR_GPIOAEN;

	// SCK --> PA5 --> AF --> 50 MHz
	GPIOA->MODER |= GPIO_MODER_MODER5_1;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_0;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_1;
	
	// MOSI --> PA7 --> AF --> 50 MHz
	GPIOA->MODER |= GPIO_MODER_MODER7_1;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7_0;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7_1;

	// MISO --> PA6 --> Floating Input --> Input Pull up
	GPIOA->MODER |= GPIO_MODER_MODER6_1;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6_0;
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6_1;
	//GPIOA->PUPDR |= GPIO_PUPDR_PUPDR6_0;
	//AFRL -->  0b01010101010100000000000000000000
	GPIOA->AFR[0] |= 1431306240;
	// SS --> PE3 --> Std Output
	GPIOE->MODER |= GPIO_MODER_MODER3_0;
	GPIOE->BSRR |= GPIO_BSRR_BS_3;
}

uint16_t spi1_write(uint8_t value) {
	while ((SPI1->SR & SPI_SR_TXE) == 0);// tansmit buffer not empty
	GPIOE->BSRR |= GPIO_BSRR_BR_3;
#if SPI1_DATA_SIZE < 9
	__IO uint8_t *spidr = (__IO uint8_t *)&SPI1->DR;
	*spidr = value;
#else
	SPI1->DR = value;
#endif
	while ((SPI1->SR & SPI_SR_RXNE) == 0);// receive buffer not empty
	GPIOE->BSRR |= GPIO_BSRR_BS_3;
	volatile uint16_t s = 0;
	while ((SPI1->SR & SPI_SR_RXNE) == 1)
		s = SPI1->DR;
	return s;
}

void spi1_write_string(char* text) {
	while (*text != '\0') {
		spi1_write((uint8_t)*text);
		text++;
	}
}
