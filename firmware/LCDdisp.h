//AQM0802 LCD module functions
//LCDdisp.h
//Copyright (C) Ryuta Mizutani 2016

#ifndef _LCDDISP_H_
#define _LCDDISP_H_

//Prototypes

void delay (unsigned int istep);
void I2Cinit(void);
void I2Cstart(void);
void I2Cstop(void);
unsigned char I2Cwrite(unsigned char ucdata);
unsigned char LCDcmd(unsigned char cmd);
void LCDoff(void);
void LCDinit();
void LCDclear(void);
void LCDputc(char c);
void LCDposition(unsigned char ucPos);
void LCDprint(char* buf);
void LCDprintHex(unsigned int iout, unsigned char ucdigit);
void LCDprintDec(unsigned long ulOut, unsigned char ucDigit);

#endif
