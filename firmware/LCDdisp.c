//AQM0802 LCD module functions
//
//Copyright (C) Ryuta Mizutani 2016

#if defined(__18F2550)
	#include <p18F2550.h>
#elif defined(__18F2553)
	#include <p18F2553.h>
#elif defined(__18F14K50)
	#include <p18F14K50.h>
#endif

//LCD ports
#define SDA_TRIS TRISBbits.TRISB0
#define SDA_PORT PORTBbits.RB0
#define SCL_TRIS TRISBbits.TRISB1
#define SCL_PORT PORTBbits.RB1
#define VDD_TRIS TRISBbits.TRISB3
#define VDD_PORT PORTBbits.RB3
//slave addr
#define S_ADDR	0x7c

//170320
//void delay (unsigned int istep) {
//	unsigned int i;
//	for (i=istep; i>0; i--) {}
//}

void delay (unsigned int istep) {
	unsigned int i;
	for (i=0; i<istep; i++) {}
}

void I2Cinit(void){
	VDD_TRIS = 0;
	VDD_PORT = 1;//VDD ON

	SDA_TRIS = 1;//SDA pull-up
	SCL_TRIS = 1;//SCL pull-up
}

void I2Cstart(void){
	SDA_TRIS = 1;//SDA pull-up
	SCL_TRIS = 1;//SCL pull-up
	delay(1);
	SDA_PORT = 0; SDA_TRIS = 0;// SDA to GND
	delay(1);
	SCL_PORT = 0; SCL_TRIS = 0;// SCL to GND
}

void I2Cstop(void){
	SCL_PORT = 0; SCL_TRIS = 0;// SCL to GND
	SDA_PORT = 0; SDA_TRIS = 0;// SDA to GND
	delay(1);
	SCL_TRIS = 1;//SCL pull-up
	delay(1);
	SDA_TRIS = 1;//SDA pull-up
}

unsigned char I2Cwrite(unsigned char ucdata){
	unsigned char i;

	SDA_TRIS = 0;//SDA MCU==>LCD
	SCL_TRIS = 0;//SCL MCU==>LCD
	for (i=0; i<8; i++){
		if (ucdata & 0x80) SDA_PORT = 1;
		else SDA_PORT = 0;
		_asm
			NOP
		_endasm
		SCL_PORT = 1;
		delay(1);
		SCL_PORT = 0;
		if (i<7) ucdata = ucdata << 1;
	}
	SDA_TRIS = 1;//SDA MCU<==LCD
	_asm
		NOP
	_endasm
	SCL_PORT = 1;
	delay(1);
	i = SDA_PORT;//get status
	SCL_PORT = 0;
	return i;
}

unsigned char LCDcmd(unsigned char cmd){
	I2Cstart();
	if (I2Cwrite(S_ADDR)) goto err;
	if (I2Cwrite(0b10000000)) goto err;
	if (I2Cwrite(cmd)) goto err;
	I2Cstop();
	delay(30);
	return 0;
err:
	I2Cstop();
	return 1;
}

void LCDoff(void){
	SDA_TRIS = 1;//SDA pull-up
	SCL_TRIS = 1;//SCL pull-up

	VDD_PORT = 0;//VDD OFF
	delay(30);
	VDD_TRIS = 1;

//	SDA_PORT = 0;
//	SDA_TRIS = 0;
//	SCL_PORT = 0;
//	SCL_TRIS = 0;
}

void LCDinit(){
	I2Cinit();
	delay(30000);
	LCDcmd(0x38);	//function set, 8 bit interface, 2 lines, single height
	LCDcmd(0x39);	//function set, instruction table 1
	LCDcmd(0x14);	//internal osc freq, 1/5 bias, F2-F0 = 100
	LCDcmd(0x70);	//contrast set, C3-C0 = 0000
	LCDcmd(0x52);	//contrast set, icon off, booster off, C5-C4 = 10
	LCDcmd(0x6C);	//follower control, follower on, RAB2-RAB0 = 100
	delay(30000);
	LCDcmd(0x38);	//function set, instrction table 0
	LCDcmd(0x0c);	//display control  display on / cursor off
	LCDcmd(0x01);	//clear display
	delay(30000);
}

void LCDclear(void) {
	LCDcmd(0x01);	//clear display
	delay(1000);
}

void LCDputc(char c){
	I2Cstart();
	if (I2Cwrite(S_ADDR)) goto err;
	I2Cwrite(0b11000000);
	I2Cwrite(c);
err:
	I2Cstop();
	delay(30);
}

//ucPos=0x0`0x7, 0x40`0x47
void LCDposition(unsigned char ucPos) {
	LCDcmd(ucPos | 0x80);
}

void LCDprint(char* buf) {
	int i;
	for (i=0; i<8; i++) {
		if (buf[i] == 0) break;
		LCDputc(buf[i]);
	}
}

void LCDprintHex(unsigned int iout, unsigned char ucdigit) {
	char buf;
	int i;
	for (i=3; i>=0; i--) {
		buf = (iout >> (i * 4)) & 0x000f;
		if (buf < 10) buf += 0x30; else buf += 0x37;
		if (i < ucdigit) LCDputc(buf);
	}
}

void LCDprintDec(unsigned long ulOut, unsigned char ucDigit) {
	int i;
	unsigned long ulDec = 1;
	char buf;

	if (ucDigit == 0) return;
	for (i=0; i<ucDigit; i++) {
		ulDec *= 10;
	}
	ulOut = ulOut % ulDec;
	for (i=0; i<ucDigit; i++) {
		ulDec /= 10;
		buf = ulOut / ulDec + 0x30;
		ulOut = ulOut % ulDec;
		LCDputc(buf);
	}
}

/*legacy
void LCDinit(void) {
	int i;
	//init LCD port
	TRISBbits.TRISB7 = 0;
	TRISBbits.TRISB6 = 0;
	TRISBbits.TRISB5 = 0;
	TRISBbits.TRISB4 = 0;
	TRISBbits.TRISB3 = 0;
	TRISBbits.TRISB2 = 0;
	//RS = 0, STB = 1
	PORTB = LATB & 0xfb;//RB2=0
	PORTB = LATB | 0x08;//RB3=1
	delay(30000);
	//////
	PORTB = LATB & 0x0f;
	PORTB = LATB | 0x30;//RB7-4=0011
	delay(1000);
	PORTB = LATB & 0xf7;//RB3=0
	delay(10000);
	PORTB = LATB | 0x08;//RB3=1
	delay(1000);
	PORTB = LATB & 0xf7;//RB3=0
	delay(10000);
	PORTB = LATB | 0x08;//RB3=1
	delay(1000);
	PORTB = LATB & 0xf7;//RB3=0
	delay(10000);
	PORTB = LATB | 0x08;//RB3=1
	//////
	//4 bit mode
	PORTB = LATB & 0x0f;
	PORTB = LATB | 0x20;//RB7-4=0010
	delay(10);
	PORTB = LATB & 0xf7;//RB3=0
	delay(10);
	PORTB = LATB | 0x08;//RB3=1
	//4 bit mode, 2 lines, 5x7 dots
	LCDoutput(0, 0x28);
	//increment mode, shift off
	LCDoutput(0, 0x06);
	//display on, cursor on, blink on
	LCDcursor(1);
	//
	LCDoutput(0, 64);//110711 CGRAM address 0
	LCDoutput(1, 0B01110);//0
	LCDoutput(1, 0B11011);//1
	LCDoutput(1, 0B10001);//2
	LCDoutput(1, 0B10001);//3
	LCDoutput(1, 0B10001);//4
	LCDoutput(1, 0B10001);//5
	LCDoutput(1, 0B11111);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B01110);//0
	LCDoutput(1, 0B11011);//1
	LCDoutput(1, 0B10001);//2
	LCDoutput(1, 0B10001);//3
	LCDoutput(1, 0B10001);//4
	LCDoutput(1, 0B11111);//5
	LCDoutput(1, 0B11111);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B01110);//0
	LCDoutput(1, 0B11011);//1
	LCDoutput(1, 0B10001);//2
	LCDoutput(1, 0B10001);//3
	LCDoutput(1, 0B11111);//4
	LCDoutput(1, 0B11111);//5
	LCDoutput(1, 0B11111);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B01110);//0
	LCDoutput(1, 0B11011);//1
	LCDoutput(1, 0B10001);//2
	LCDoutput(1, 0B11111);//3
	LCDoutput(1, 0B11111);//4
	LCDoutput(1, 0B11111);//5
	LCDoutput(1, 0B11111);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B01110);//0
	LCDoutput(1, 0B11111);//1
	LCDoutput(1, 0B11111);//2
	LCDoutput(1, 0B11111);//3
	LCDoutput(1, 0B11111);//4
	LCDoutput(1, 0B11111);//5
	LCDoutput(1, 0B11111);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B00001);//0
	LCDoutput(1, 0B00011);//1
	LCDoutput(1, 0B11101);//2
	LCDoutput(1, 0B10101);//3
	LCDoutput(1, 0B11101);//4
	LCDoutput(1, 0B00011);//5
	LCDoutput(1, 0B00001);//6
	LCDoutput(1, 0B00000);//7
	//
	LCDoutput(1, 0B10100);//7
	LCDoutput(1, 0B01000);//7
	LCDoutput(1, 0B10101);//0
	LCDoutput(1, 0B00011);//1
	LCDoutput(1, 0B11101);//2
	LCDoutput(1, 0B11101);//4
	LCDoutput(1, 0B00011);//5
	LCDoutput(1, 0B00001);//6
	//
	LCDoutput(0, 0x02);//return home
}
*/

