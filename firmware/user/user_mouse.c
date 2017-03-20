/*
Two-factor authentification dongle
Mizutani Lab (c) 2017
*/

/*********************************************************************
 *
 *                Microchip USB C18 Firmware - Mouse Demo
 *
 *********************************************************************
 * FileName:        user_mouse.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the ìCompanyÅE for its PICmicroÆ Microcontroller is intended and
 * supplied to you, the Companyís customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN ìAS ISÅECONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       11/19/04    Original.
 ********************************************************************/

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include <usart.h>
#include "system\typedefs.h"

#include "system\usb\usb.h"

#include "io_cfg.h"             // I/O pin mapping
#include "user\user_mouse.h"
#include "LCDdisp.h"
#include "clongint.h"

/** V A R I A B L E S ********************************************************/
#pragma udata

unsigned long long ullUTC = 0;
unsigned long long ullUTCorg = 0;
double dUTCdelta = 0;
unsigned char ucKeyIdx = 0;
unsigned long long ullUTCwake = 0;

char buffer[65];
unsigned char ucDisp = DISP_PERIOD;
unsigned char pucSW[4];
unsigned char ucTick = 0;
BOOL bFindNextEntry = FALSE;
BOOL bSleepMode = FALSE;
//#define usb_bus_sense_int 0
#define usb_bus_sense_int usb_bus_sense

//encryption
const unsigned char icm[] = {189,238,198,25,191,14,143,88,47,65,228,130,110,4,96,139,99,89,171,21,38,0};
const unsigned char clenm = 22;
const unsigned char ice[] = {63,39,93,11,141,96,88,52,100,215,19,155,35,185,5,255,33,172,41,15};
const unsigned char clene = 20;
const unsigned char icd[] = {131,2,0};
const unsigned char clend = 3;
struct CLongInt ib, im, id;

/** P R I V A T E  P R O T O T Y P E S ***************************************/


/** D E C L A R A T I O N S **************************************************/
#pragma code

void WriteEEPROM(unsigned char caddr, unsigned char cdata) {
	EEADR = caddr;
	EEDATA = cdata;
	EECON1bits.CFGS = 0;
	EECON1bits.EEPGD = 0;
	EECON1bits.FREE = 0;
	EECON1bits.WREN = 1;
	INTCONbits.GIE = 0;
	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;
	INTCONbits.GIE = 1;
	EECON1bits.WREN = 0;
	while (EECON1bits.WR == 1) continue;
}

void ReadEEPROM(unsigned char caddr, unsigned char* pcdata) {
	EEADR = caddr;
	EECON1bits.CFGS = 0;
	EECON1bits.EEPGD = 0;
	EECON1bits.FREE = 0;
	EECON1bits.RD = 1;
	*pcdata = EEDATA;
}

void UserInit(void)
{
	unsigned char uc0, uc1;
	//#pragma config PBADEN = OFF
	//When PBADEN = 1, PCFG<3:0> = 0000; when PBADEN = 0, PCFG<3:0> = 0111.
	ADCON1bits.PCFG = 7;
	//unused open ports
	TRISAbits.TRISA4 = 0;
	PORTAbits.RA4 = 0;
	TRISBbits.TRISB5 = 0;
	PORTBbits.RB5 = 0;
	TRISBbits.TRISB6 = 0;
	PORTBbits.RB6 = 0;
	TRISBbits.TRISB7 = 0;
	PORTBbits.RB7 = 0;
	TRISCbits.TRISC2 = 0;
	PORTCbits.RC2 = 0;
	TRISCbits.TRISC6 = 0;
	PORTCbits.RC6 = 0;
	TRISCbits.TRISC7 = 0;
	PORTCbits.RC7 = 0;
	//INTRC
	//OSCCONbits.IRCF = 4;//1 MHz: default
	//OSCCONbits.IRCF = 5;//2 MHz
	//OSCCONbits.IRCF = 6;//4 MHz
	OSCCONbits.IRCF = 7;//8 MHz

	ucKeyIdx = -1;
	for (uc0=0; uc0<MAX_ENTRY; uc0++) {
		ReadEEPROM(MAX_ENTRY * 2 + uc0 * LABELSIZE, &uc1);
		if (uc1 != 0xff) {ucKeyIdx = uc0; break;}
	}
	if (ucKeyIdx < 0) ucKeyIdx = 0;

	//SW
	TRISBbits.TRISB2 = 1;
	INTCON2bits.RBPU = 1;//disable pull-ups
	for (uc0=0; uc0<4; uc0++) {pucSW[uc0] = 1;}
	INTCON3bits.INT2IP = 0;//low priority
	INTCON2bits.INTEDG2 = 0;//int on falling edge
	INTCON3bits.INT2IE = 1;//enable RB2 interrupt

	//LCD
	LCDinit();

	//Interrupt
	INTCONbits.GIE = 0;//Disable interrupts
	RCONbits.IPEN = 1;//enable interrupt priority (disabled at POR)
	IPR2bits.USBIP = 1;//enable USBIP (same with POR)

	T1CON = 0b00001110;// 1/1prescale,T1osc=on,sync=off,T1OSC clock, stop
	TMR1H = 0x80;
	TMR1L = 0x00;
	PIE1bits.TMR1IE = 1;//enable interrupt
//	IPR1bits.TMR1IP = 0;//low priority
	IPR1bits.TMR1IP = 1;//high priority
	T1CONbits.TMR1ON = 1;//start timer1

	INTCONbits.PEIE = 1;//Enable all peripheral interrupts
	INTCONbits.GIE = 1;//Enable all interrupts
}//end UserInit

/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/
void ProcessIO(void)
{   
	unsigned long long ullUTCdiv;
	unsigned long long ullUTCreal;
	unsigned long ulCode, ulUTCnorm, ul2;
	unsigned long long ullUTCset;
	unsigned char pcCounter[8];
	unsigned char pcKey[KEYSIZE];
	unsigned char ucKeyLength;
	char msg[8];
	const char pcBlankMsg[9] = "ALT 2FA";
	const char pcRevMsg[9] = "  170320";
	const char pcWaitMsg[9] = "Wait..  ";
	unsigned int i, j;
	int ix;
	unsigned char uc0, uc1, uc2;
	BOOL bTxUSB = TRUE;
	BOOL bSwSense = FALSE;

//Get switch
	ucTick++;
	uc0 = ucTick & 0x03;
	uc1 = (ucTick-1) & 0x03;
	uc2 = (ucTick-2) & 0x03;
	pucSW[uc0] = PORTBbits.RB2;
	bSwSense = FALSE;
	if (usb_bus_sense == 1) {
		if ( ((pucSW[uc0] == 0)&&(pucSW[uc1] == 0)&&(pucSW[uc2] == 1)) || bFindNextEntry ) bSwSense = TRUE;
	} else {
		if ( ((pucSW[uc0] == 0)&&(pucSW[uc1] == 1)) || bFindNextEntry ) bSwSense = TRUE;
	}
	if (bSwSense) {
		uc2 = ucKeyIdx;
		uc1 = 0xff;
		do {
			ucKeyIdx++;
			if (ucKeyIdx >= MAX_ENTRY) ucKeyIdx = 0;
			if (uc2 == ucKeyIdx) break;
			ReadEEPROM(MAX_ENTRY * 2 + ucKeyIdx * LABELSIZE, &uc1);
		} while (uc1 == 0xff);
		if (uc2 != ucKeyIdx) ucDisp = DISP_PERIOD;
		bFindNextEntry = FALSE;
		ullUTCwake = ullUTC;
	}

//Sleep
	if (usb_bus_sense == 1) ullUTCwake = ullUTC;
	if (ullUTC - ullUTCwake >= WAKE_LIMIT) {
		bSleepMode = TRUE;
		LCDoff();
//		INTCON3bits.INT2IE = 1;//enable RB2 interrupt
		UCONbits.USBEN = 0;
		OSCCONbits.IDLEN = 0;
		while (bSleepMode) {
			_asm
			sleep
			_endasm
		}
		ullUTCwake = ullUTC;
//		INTCON3bits.INT2IE = 0;//disable
		UCONbits.USBEN = 1;
		LCDinit();
	}

//Disp
	if (ucDisp == DISP_PERIOD) {
		ucDisp = 0;
		ullUTCreal = ullUTC + (long)((double)(ullUTC - ullUTCorg) * dUTCdelta);
		ullUTCdiv = ullUTCreal / 30;
		ulUTCnorm = ullUTCreal % 30;

		for (uc0=0; uc0<LABELSIZE; uc0++) {
			ReadEEPROM(MAX_ENTRY * 2 + ucKeyIdx * LABELSIZE + uc0, &uc1);
			msg[uc0] = uc1;
		}
		if (msg[0] == 0xff) {//blank entry
			LCDposition(0x00);
			LCDprint(pcBlankMsg);
			LCDposition(0x40);
			LCDprint(pcRevMsg);
		} else {
			//show auth code
			ReadEEPROM(ucKeyIdx, &ucKeyLength);
			for (uc0=0; uc0<ucKeyLength; uc0++) {
				ReadEEPROM(MAX_ENTRY * (2 + LABELSIZE) + ucKeyIdx * KEYSIZE + uc0, &uc1);
				pcKey[uc0] = uc1;
			}
			for (ix=7; ix>=0; ix--) {
				pcCounter[ix] = (unsigned char)(ullUTCdiv & 0xff);
				ullUTCdiv = ullUTCdiv >> 8;
			}
			LCDposition(0x40);
			if (hmac_sha1(&ulCode, pcKey, ucKeyLength, pcCounter, 8) == 0) {
				LCDprintDec(ulCode, 6);
				LCDputc(' ');
				if (30 - ulUTCnorm < 10) {
					LCDprintDec(30 - ulUTCnorm, 1);
				} else {
					LCDputc(' ');
				}
			}
			//show label
			LCDposition(0x00);
			LCDprint(msg);
		}//if (msg[0] == 0xff)
	}
    if(UCONbits.USBEN == 0) {               // Is the module off?
		OSCCONbits.SCS = 2;//170301 internal RC osc
	}

	if (usb_device_state != CONFIGURED_STATE) return;//110703

	//#define EP1_BUF_SIZE 64
	if (HIDRxReport(buffer, EP1_BUF_SIZE) > 0) // USB receive buffer has data
    {
		if (buffer[0] == 'T') {//time
			ullUTCset = 0;
			for (i=1; i<=8; i++) {
				ullUTCset |= ((unsigned char)buffer[i]);
				if (i < 8) ullUTCset = ullUTCset << 8;
			}
			if ((ullUTCorg == 0)||(buffer[9] == 0x01)) {
				ullUTC = ullUTCset;
				dUTCdelta = 0.0;
				ullUTCorg = ullUTCset;
			} else {
				dUTCdelta = ullUTCset - ullUTC;
				dUTCdelta = dUTCdelta / (double)(ullUTC - ullUTCorg);
			}
			ullUTCwake = ullUTC;
			ucDisp = DISP_PERIOD;
		} else if (buffer[0] == 'K') {
			//1:index, 2:native code length, 3:encrypted length, 4-11:label, 12-:code
			if ((buffer[2] <= KEYSIZE)&&(buffer[1] < MAX_ENTRY)) {
				WriteEEPROM(buffer[1], buffer[2]);//addr #keybytes
				for (uc0=0; uc0<LABELSIZE; uc0++) {
					WriteEEPROM(uc0 + MAX_ENTRY * 2 + buffer[1] * LABELSIZE, buffer[uc0 + 4]);//label
				}
				if (buffer[3] != 0) {
					LCDposition(0x40);
					LCDprint(pcWaitMsg);
					//decode crypt
					LongIntInit(&im);
					LongIntSet(&im, icm, clenm);
					LongIntInit(&id);
					LongIntSet(&id, icd, clend);
					LongIntInit(&ib);
					for (uc0=0; uc0<buffer[3]; uc0++) {ib.pElem[uc0] = buffer[12+uc0];}
					ib.nElem = buffer[3];
					LongIntPowerMod(&ib, &id, &im);
					for (uc0=0; uc0<buffer[2]; uc0++) {
						WriteEEPROM(uc0 + MAX_ENTRY * (2 + LABELSIZE) + buffer[1] * KEYSIZE, ib.pElem[uc0]);//key
					}
				} else {
					for (uc0=0; uc0<buffer[2]; uc0++) {
						WriteEEPROM(uc0 + MAX_ENTRY * (2 + LABELSIZE) + buffer[1] * KEYSIZE, buffer[uc0 + 12]);//key
					}
				}
				if (buffer[uc0+4] == 0xff) bFindNextEntry = TRUE;
				ucKeyIdx = buffer[1];
				ucDisp = DISP_PERIOD;
			}
			for (i=0; i<EP1_BUF_SIZE; i++) {buffer[i] = 0;}
		} else if (buffer[0] == 'L') {
			for (uc2=0; uc2<MAX_ENTRY; uc2++) {
				for (uc0=0; uc0<LABELSIZE; uc0++) {
					ReadEEPROM(MAX_ENTRY * 2 + uc2 * LABELSIZE + uc0, &uc1);
					buffer[uc2 * LABELSIZE + uc0] = uc1;
				}
			}
		} else if (buffer[0] == 'C') {//encryption keys
			buffer[0] = clenm;
			for (uc0=0; uc0<clenm; uc0++) {buffer[uc0+1] = icm[uc0];}
			buffer[clenm + 1] = clene;
			for (uc0=0; uc0<clene; uc0++) {buffer[uc0+clenm+2] = ice[uc0];}
		} else if (buffer[0] == 'D') {//legacy
			buffer[1] = 0x55;
			buffer[2] = 0x55;
			buffer[3] = 0x55;
			buffer[4] = 0x55;
			buffer[5] = 0x03;
			buffer[6] = 0xff;
		}
		if (bTxUSB) {
	        while(mHIDTxIsBusy()){}                 // blocking
    	    HIDTxReport(buffer, EP1_BUF_SIZE);   // transmit packet
		}
	}
    
}//end ProcessIO

/** EOF user_mouse.c *********************************************************/
