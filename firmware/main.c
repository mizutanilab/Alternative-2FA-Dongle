/*
Two-factor authentification dongle
Mizutani Lab (c) 2017
*/

/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        main.c
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
#include "system\typedefs.h"                        // Required
#include "system\usb\usb.h"                         // Required
#include "io_cfg.h"                                 // Required

#include "system\usb\usb_compile_time_validation.h" // Optional
#include "user\user_mouse.h"                        // Modifiable

/** V A R I A B L E S ********************************************************/
#pragma udata

/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBTasks(void);

void YourHighPriorityISRCode();
void YourLowPriorityISRCode();

extern unsigned long long ullUTC;
extern unsigned char ucDisp;
extern BOOL bSleepMode;
extern unsigned long long ullUTCwake;
//#define usb_bus_sense_int 0
#define usb_bus_sense_int usb_bus_sense


/** V E C T O R  R E M A P P I N G *******************************************/
//110626
//extern void _startup (void);        // See c018i.c in your C18 compiler dir
//#pragma code _RESET_INTERRUPT_VECTOR = 0x000800
//void _reset (void)
//{
//    _asm goto _startup _endasm
//}
/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
	
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
		_asm
		goto YourHighPriorityISRCode
		_endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto YourLowPriorityISRCode _endasm
	}
	
	#pragma code
	
	#pragma tmpdata ISRtmpdata	// use ISRtmpdata
	//These are your actual interrupt handling routines.
	#pragma interrupt YourHighPriorityISRCode nosave=section(".tmpdata"), TBLPTRL, TBLPTRH, TBLPTRU, TABLAT, PCLATH, PCLATU
	void YourHighPriorityISRCode()
	{
		if (PIR1bits.TMR1IF) {
			_asm bsf TMR1H,7,0 _endasm
			//TMR1H = 0x80;
			//TMR1L = 0x00;
			PIR1bits.TMR1IF = 0;
			ullUTC++;
			ucDisp = DISP_PERIOD;
			if (usb_bus_sense_int) {
		        if(UCONbits.USBEN == 0) {               // Is the module off?
					OSCCONbits.SCS = 0;//170301 primary osc
		            USBModuleEnable();                  // When off, enable it
				}
				bSleepMode = FALSE;
			}
		}
	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 

	#pragma tmpdata	//use the default .tmpdata section
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
		if (INTCON3bits.INT2IF) {
			INTCON3bits.INT2IF = 0;
			bSleepMode = FALSE;
			ucDisp = DISP_PERIOD;
			ullUTCwake = ullUTC;
			OSCCONbits.SCS = 0;//170301 primary osc
		}
	}	//This return will be a "retfie", since this is in a #pragma interruptlow section 

#endif //of "#if defined(__18CXX)"

/** D E C L A R A T I O N S **************************************************/
#pragma code
/******************************************************************************
 * Function:        void main(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Main program entry point.
 * Note:            None
 *****************************************************************************/
void main(void)
{
	unsigned char ucGIE;

    InitializeSystem();
    while(1)
    {
		ucGIE = INTCONbits.GIE;
		INTCONbits.GIE = 0;//disable all interrupts
        USBTasks();         // USB Tasks
		INTCONbits.GIE = ucGIE;

        ProcessIO();        // See user\user.c & .h
    }//end while
}//end main

/******************************************************************************
 * Function:        static void InitializeSystem(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        InitializeSystem is a centralize initialization routine.
 *                  All required USB initialization routines are called from
 *                  here.
 *                  User application initialization routine should also be
 *                  called from here.                  
 * Note:            None
 *****************************************************************************/
static void InitializeSystem(void)
{
    ADCON1 |= 0x0F;                 // Default all pins to digital
    
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See io_cfg.h
    #endif
    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;
    #endif
    
    mInitializeUSBDriver();         // See usbdrv.h
    
    UserInit();                     // See user.c & .h

}//end InitializeSystem

/******************************************************************************
 * Function:        void USBTasks(void)
 * PreCondition:    InitializeSystem has been called.
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Service loop for USB tasks.
 * Note:            None
 *****************************************************************************/
void USBTasks(void)
{
    /*
     * Servicing Hardware
     */
    USBCheckBusStatus();                    // Must use polling method
    if(UCFGbits.UTEYE!=1)
        USBDriverService();                 // Interrupt or polling method

}// end USBTasks

//configuration
	//#pragma config FOSC = HS
	#pragma config FOSC = HSPLL_HS
	#pragma config PLLDIV = 5 //20 MHz crystal
	//#pragma config CPUDIV = OSC4_PLL6 //FOSC=HS: 5 MHz, FOSC=HSPLL: 16 MHz
	#pragma config CPUDIV = OSC2_PLL3 //FOSC=HS: 10 MHz, FOSC=HSPLL: 32 MHz
	//#pragma config CPUDIV = OSC1_PLL2 //FOSC=HS: 20 MHz, FOSC=HSPLL: 48 MHz
	#pragma config USBDIV = 2 //20 MHz crystal

	#pragma config FCMEN = OFF
	#pragma config IESO = OFF 
	
	#pragma config PWRT = OFF 
	#pragma config BOR = OFF 
	//#pragma config BORV = 28 
	#pragma config BORV = 3
	#pragma config VREGEN = ON 
	
	#pragma config WDT = OFF 
	//#pragma config WDTPS = 1 
	#pragma config WDTPS = 32768
	
	#pragma config MCLRE = OFF 
	//sleep current = 11 uA
	#pragma config LPT1OSC = OFF
	//sleep current = 6.6 uA (MCU must be shielded)
	//#pragma config LPT1OSC = ON
	#pragma config PBADEN = OFF 
	//#pragma config CCP2MX = OFF
	
	//#pragma config STVREN = OFF 
	#pragma config STVREN = ON
	#pragma config LVP = OFF 
//	#pragma config ICPRT = OFF 
	#pragma config XINST = OFF 
	#pragma config DEBUG = OFF
	
//* these must be set ON for release
	#pragma config CP0 = ON 
	#pragma config CP1 = ON
	#pragma config CP2 = ON 
	#pragma config CP3 = ON
	#pragma config CPB = ON
	#pragma config WRT0 = ON 
	#pragma config WRT1 = ON
	#pragma config WRT2 = ON 
	#pragma config WRT3 = ON
	#pragma config WRTB = ON
	#pragma config WRTC = ON
//EEPROM protection
	#pragma config CPD = ON
///*///
/*code for debug
	#pragma config CP0 = OFF 
	#pragma config CP1 = OFF
	#pragma config CP2 = OFF 
	#pragma config CP3 = OFF
	#pragma config CPB = OFF
	#pragma config WRT0 = OFF 
	#pragma config WRT1 = OFF
	#pragma config WRT2 = OFF 
	#pragma config WRT3 = OFF
	#pragma config WRTB = OFF
	#pragma config WRTC = OFF
//EEPROM protection
	#pragma config CPD = OFF
///*///
	#pragma config EBTR0 = OFF 
	#pragma config EBTR1 = OFF
	#pragma config EBTR2 = OFF 
	#pragma config EBTR3 = OFF
	#pragma config EBTRB = OFF
//EEPROM protection
	#pragma config WRTD = OFF

/** EOF main.c ***************************************************************/
