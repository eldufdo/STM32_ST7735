#include <string.h> // For memcpy
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_conf.h"
#include "delay.h"
#include "st7735.h"
#include "font5x7.h"
#include "img.h"

uint16_t scr_width;
uint16_t scr_height;


void ST7735_write(uint8_t data) {
	while (SPI_I2S_GetFlagStatus(SPI_PORT,SPI_I2S_FLAG_TXE) == RESET);
	//SPI_I2S_SendData16(SPI_PORT,data);
	SPI_SendData8(SPI_PORT,data);
}

void ST7735_cmd(uint8_t cmd) {
	A0_L();
	ST7735_write(cmd);
}

void ST7735_data(uint8_t data) {
	A0_H();
	ST7735_write(data);
	while (SPI_I2S_GetFlagStatus(SPI_PORT,SPI_I2S_FLAG_BSY) == SET);
}

uint16_t RGB565(uint8_t R,uint8_t G,uint8_t B) {
	return ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
}

void ST7735_Init(void) {

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	// Configure and enable SPI
	SPI_InitTypeDef SPI;
	SPI.SPI_Mode = SPI_Mode_Master;
	SPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI.SPI_CPOL = SPI_CPOL_Low;
	SPI.SPI_CPHA = SPI_CPHA_1Edge;
	SPI.SPI_CRCPolynomial = 7;
	SPI.SPI_DataSize = SPI_DataSize_8b;
	SPI.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI.SPI_NSS = SPI_NSS_Soft;

	SPI_Init(SPI_PORT,&SPI);
	// NSS must be set to '1' due to NSS_Soft settings (otherwise it will be Multimaster mode).
	SPI_NSSInternalSoftwareConfig(SPI_PORT,SPI_NSSInternalSoft_Set);
	SPI_Cmd(SPI_PORT,ENABLE);

	GPIO_InitTypeDef PORT;
	PORT.GPIO_Mode = GPIO_Mode_OUT;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOA->AFR[0] |= 1431306240;

	// Configure SPI pins
	PORT.GPIO_Pin = SPI_SCK_PIN | SPI_MOSI_PIN;
	PORT.GPIO_Mode = GPIO_Mode_AF;
	PORT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	PORT.GPIO_OType = GPIO_OType_PP;

	GPIO_Init(SPI_GPIO_PORT,&PORT);
	PORT.GPIO_Mode = GPIO_Mode_OUT;

	PORT.GPIO_Pin = ST7735_CS_PIN;
	GPIO_Init(ST7735_CS_PORT,&PORT);
	PORT.GPIO_Pin = ST7735_RST_PIN;
	GPIO_Init(ST7735_RST_PORT,&PORT);
	PORT.GPIO_Pin = ST7735_A0_PIN;
	GPIO_Init(ST7735_A0_PORT,&PORT);

	// Reset display
	CS_H();
	RST_H();
	Delay_ms(5);
	RST_L();
	Delay_ms(5);
	RST_H();
	CS_H();
	Delay_ms(5);
	CS_L();
	ST7735_cmd(0x11); // Sleep out & booster on
	Delay_ms(255); // Datasheet says what display wakes about 120ms (may be much faster actually)

	ST7735_cmd(0xb1);   // In normal mode (full colors):
	A0_H();
	ST7735_write(0x05); //   RTNA set 1-line period: RTNA2, RTNA0
	ST7735_write(0x3c); //   Front porch: FPA5,FPA4,FPA3,FPA2
	ST7735_write(0x3c); //   Back porch: BPA5,BPA4,BPA3,BPA2

	ST7735_cmd(0xb2);   // In idle mode (8-colors):
	A0_H();
	ST7735_write(0x05); //   RTNB set 1-line period: RTNAB, RTNB0
	ST7735_write(0x3c); //   Front porch: FPB5,FPB4,FPB3,FPB2
	ST7735_write(0x3c); //   Back porch: BPB5,BPB4,BPB3,BPB2

	ST7735_cmd(0xb3);   // In partial mode + full colors:
	A0_H();
	ST7735_write(0x05); //   RTNC set 1-line period: RTNC2, RTNC0
	ST7735_write(0x3c); //   Front porch: FPC5,FPC4,FPC3,FPC2
	ST7735_write(0x3c); //   Back porch: BPC5,BPC4,BPC3,BPC2
	ST7735_write(0x05); //   RTND set 1-line period: RTND2, RTND0
	ST7735_write(0x3c); //   Front porch: FPD5,FPD4,FPD3,FPD2
	ST7735_write(0x3c); //   Back porch: BPD5,BPD4,BPD3,BPD2

	ST7735_cmd(0xB4);   // Display dot inversion control:
	ST7735_data(0x03);  //   NLB,NLC

	ST7735_cmd(0x3a);   // Interface pixel format
	//	ST7735_data(0x03);  // 12-bit/pixel RGB 4-4-4 (4k colors)
	ST7735_data(0x05);  // 16-bit/pixel RGB 5-6-5 (65k colors)
	//	ST7735_data(0x06);  // 18-bit/pixel RGB 6-6-6 (256k colors)

	//	ST7735_cmd(0x36);   // Memory data access control:
	//   MY MX MV ML RGB MH - -
	//	ST7735_data(0x00);  //   Normal: Top to Bottom; Left to Right; RGB
	//	ST7735_data(0x80);  //   Y-Mirror: Bottom to top; Left to Right; RGB
	//	ST7735_data(0x40);  //   X-Mirror: Top to Bottom; Right to Left; RGB
	//	ST7735_data(0xc0);  //   X-Mirror,Y-Mirror: Bottom to top; Right to left; RGB
	//	ST7735_data(0x20);  //   X-Y Exchange: X and Y changed positions
	//	ST7735_data(0xA0);  //   X-Y Exchange,Y-Mirror
	//	ST7735_data(0x60);  //   X-Y Exchange,X-Mirror
	//	ST7735_data(0xE0);  //   X-Y Exchange,X-Mirror,Y-Mirror

	ST7735_cmd(0x20);   // Display inversion off
	//	ST7735_cmd(0x21);   // Display inversion on

	ST7735_cmd(0x13);   // Partial mode off

	ST7735_cmd(0x26);   // Gamma curve set:
	ST7735_data(0x01);  //   Gamma curve 1 (G2.2) or (G1.0)
	//	ST7735_data(0x02);  //   Gamma curve 2 (G1.8) or (G2.5)
	//	ST7735_data(0x04);  //   Gamma curve 3 (G2.5) or (G2.2)
	//	ST7735_data(0x08);  //   Gamma curve 4 (G1.0) or (G1.8)

	ST7735_cmd(0x29);   // Display on

	CS_H();

	ST7735_Orientation(scr_normal);
}

void ST7735_Orientation(ScrOrientation_TypeDef orientation) {
	CS_L();
	ST7735_cmd(0x36); // Memory data access control:
	switch(orientation) {
	case scr_CW:
		scr_width  = scr_h;
		scr_height = scr_w;
		ST7735_data(0xA0); // X-Y Exchange,Y-Mirror
		break;
	case scr_CCW:
		scr_width  = scr_h;
		scr_height = scr_w;
		ST7735_data(0x60); // X-Y Exchange,X-Mirror
		break;
	case scr_180:
		scr_width  = scr_w;
		scr_height = scr_h;
		ST7735_data(0xc0); // X-Mirror,Y-Mirror: Bottom to top; Right to left; RGB
		break;
	default:
		scr_width  = scr_w;
		scr_height = scr_h;
		ST7735_data(0x00); // Normal: Top to Bottom; Left to Right; RGB
		break;
	}
	CS_H();
}

void ST7735_AddrSet(uint16_t XS, uint16_t YS, uint16_t XE, uint16_t YE) {
	ST7735_cmd(0x2a); // Column address set
	A0_H();
	ST7735_write(XS >> 8);
	ST7735_write(XS);
	ST7735_write(XE >> 8);
	ST7735_write(XE);

	ST7735_cmd(0x2b); // Row address set
	A0_H();
	ST7735_write(YS >> 8);
	ST7735_write(YS);
	ST7735_write(YE >> 8);
	ST7735_write(YE);

	ST7735_cmd(0x2c); // Memory write
}

uint16_t ST7735_Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void ST7735_Clear(uint16_t color) {
	uint16_t i;
	uint8_t  CH,CL;

	CH = color >> 8;
	CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(0,0,scr_width - 1,scr_height - 1);
	A0_H();
	for (i = 0; i < scr_width * scr_height; i++) {
		ST7735_write(CH);
		ST7735_write(CL);
	}
	CS_H();
}

void ST7735_Pixel(uint16_t X, uint16_t Y, uint16_t color) {
	CS_L();
	ST7735_AddrSet(X,Y,X,Y);
	A0_H();
	ST7735_write(color >> 8);
	ST7735_write((uint8_t)color);
	CS_H();
}

void ST7735_HLine(uint16_t X1, uint16_t X2, uint16_t Y, uint16_t color) {
	uint16_t i;
	uint8_t CH = color >> 8;
	uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X1,Y,X2,Y);
	A0_H();
	for (i = 0; i <= (X2 - X1); i++) {
		ST7735_write(CH);
		ST7735_write(CL);
	}
	CS_H();
}

void ST7735_VLine(uint16_t X, uint16_t Y1, uint16_t Y2, uint16_t color) {
	uint16_t i;
	uint8_t CH = color >> 8;
	uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X,Y1,X,Y2);
	A0_H();
	for (i = 0; i <= (Y2 - Y1); i++) {
		ST7735_write(CH);
		ST7735_write(CL);
	}
	CS_H();
}

void ST7735_Line(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t color) {
	int16_t dX = X2-X1;
	int16_t dY = Y2-Y1;
	int16_t dXsym = (dX > 0) ? 1 : -1;
	int16_t dYsym = (dY > 0) ? 1 : -1;

	if (dX == 0) {
		if (Y2>Y1) ST7735_VLine(X1,Y1,Y2,color); else ST7735_VLine(X1,Y2,Y1,color);
		return;
	}
	if (dY == 0) {
		if (X2>X1) ST7735_HLine(X1,X2,Y1,color); else ST7735_HLine(X2,X1,Y1,color);
		return;
	}

	dX *= dXsym;
	dY *= dYsym;
	int16_t dX2 = dX << 1;
	int16_t dY2 = dY << 1;
	int16_t di;

	if (dX >= dY) {
		di = dY2 - dX;
		while (X1 != X2) {
			ST7735_Pixel(X1,Y1,color);
			X1 += dXsym;
			if (di < 0) {
				di += dY2;
			} else {
				di += dY2 - dX2;
				Y1 += dYsym;
			}
		}
	} else {
		di = dX2 - dY;
		while (Y1 != Y2) {
			ST7735_Pixel(X1,Y1,color);
			Y1 += dYsym;
			if (di < 0) {
				di += dX2;
			} else {
				di += dX2 - dY2;
				X1 += dXsym;
			}
		}
	}
	ST7735_Pixel(X1,Y1,color);
}

void ST7735_Rect(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t color) {
	ST7735_HLine(X1,X2,Y1,color);
	ST7735_HLine(X1,X2,Y2,color);
	ST7735_VLine(X1,Y1,Y2,color);
	ST7735_VLine(X2,Y1,Y2,color);
}

void ST7735_FillRect(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t color) {
	uint16_t i;
	uint16_t FS = (X2 - X1 + 1) * (Y2 - Y1 + 1);
	uint8_t CH = color >> 8;
	uint8_t CL = (uint8_t)color;

	CS_L();
	ST7735_AddrSet(X1,Y1,X2,Y2);
	A0_H();
	for (i = 0; i < FS; i++) {
		ST7735_write(CH);
		ST7735_write(CL);
	}
	CS_H();
}

void ST7735_PutChar5x7(uint16_t X, uint16_t Y, uint8_t chr, uint16_t color,uint16_t bgcolor,uint8_t scale) {
	uint16_t i,j;
	uint8_t buffer[5];
	uint8_t CH = color >> 8;
	uint8_t CL = (uint8_t)color;
	uint8_t BGCH = bgcolor >> 8;
	uint8_t BGCL = (uint8_t)bgcolor;
	memcpy(buffer,&Font5x7[(chr - 32) * 5],5);

	CS_L();
	ST7735_AddrSet(X,Y,X + (scale * 5) - 1,Y + (scale * 7) -1);
	A0_H();
	for (j = 0; j < 7; j++) {
		for (i = 0; i < 5; i++) {
			if ((buffer[i] >> j) & 0x01) {
				uint8_t s = 0;
				for (s = 0; s < scale; s++) {
					ST7735_write(CH);
					ST7735_write(CL);
				}
			} else {
				uint8_t s = 0;
				for (s = 0; s < scale; s++) {
					ST7735_write(BGCH);
					ST7735_write(BGCL);
				}
			}
		}
	}
	CS_H();
}


void ST7735_PutStr5x7(uint8_t X, uint8_t Y, char *str, uint16_t color,uint16_t bg_color,uint8_t scale) {
	while (*str) {
		ST7735_PutChar5x7(X,Y,*str++,color,bg_color,scale);
		if (X < scr_width - (scale*5-1)) { X += (scale * 5) + 1; } else if (Y < scr_height - (scale * 7) + 1) { X = 0; Y += (scale * 7) + 1; } else { X = 0; Y = 0; }
	};
}

void ST7735_PutImg() {
	uint16_t row,col;
	uint16_t color;
	for (row = 0; row < 128; row++) {
		for (col = 0; col < 160;col++) {
			//unsigned char	 pixel_data[160 * 128 * 3 + 1];
			char r = gimp_image.pixel_data[(row*160+col)*3];
			char g = gimp_image.pixel_data[((row*160+col)*3)+1];
			char b = gimp_image.pixel_data[((row*160+col)*3)+2];
			color = ST7735_Color565(r,g,b);
			ST7735_Pixel(col,row,color);
		}
	}
}



void ST7735_drawCircleHelper( int16_t x0, int16_t y0,int16_t r, uint8_t cornername, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			ST7735_Pixel(x0 + x, y0 + y, color);
			ST7735_Pixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			ST7735_Pixel(x0 + x, y0 - y, color);
			ST7735_Pixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			ST7735_Pixel(x0 - y, y0 + x, color);
			ST7735_Pixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			ST7735_Pixel(x0 - y, y0 - x, color);
			ST7735_Pixel(x0 - x, y0 - y, color);
		}
	}
}

void ST7735_drawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	ST7735_Pixel(x0, y0+r, color);
	ST7735_Pixel(x0, y0-r, color);
	ST7735_Pixel(x0+r, y0, color);
	ST7735_Pixel(x0-r, y0, color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		ST7735_Pixel(x0 + x, y0 + y, color);
		ST7735_Pixel(x0 - x, y0 + y, color);
		ST7735_Pixel(x0 + x, y0 - y, color);
		ST7735_Pixel(x0 - x, y0 - y, color);

		ST7735_Pixel(x0 + y, y0 + x, color);
		ST7735_Pixel(x0 - y, y0 + x, color);
		ST7735_Pixel(x0 + y, y0 - x, color);
		ST7735_Pixel(x0 - y, y0 - x, color);

	}
}

void ST7735_fillCircle(int poX, int poY, int r,unsigned int color)
{
	int x = -r, y = 0, err = 2-2*r, e2;
	do {
		ST7735_VLine(poX-x,poY-y,poY+y,color);
		ST7735_VLine(poX+x,poY-y,poY+y,color);
		e2 = err;
		if (e2 <= y) {
			err += ++y*2+1;
			if (-x == y && e2 <= x) e2 = 0;
		}
		if (e2 > x) err += ++x*2+1;
	} while (x <= 0);
}

void ST7735_drawRoundRect(int16_t x, int16_t y, int16_t w,int16_t h, int16_t r, uint16_t color) {
	// smarter version
	ST7735_HLine(x+r ,x+w-r, y, color); // Top
	ST7735_HLine(x+r , x+w-r,y+h-1, color); // Bottom
	ST7735_VLine(x , y+r , y+h-r, color); // Left
	ST7735_VLine(x+w, y+r , y+h-r, color); // Right
	// draw four corners
	ST7735_drawCircleHelper(x+r , y+r , r, 1, color);
	ST7735_drawCircleHelper(x+w-r-1, y+r , r, 2, color);
	ST7735_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
	ST7735_drawCircleHelper(x+r , y+h-r-1, r, 8, color);
}
