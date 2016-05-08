// USBDemoBoard.c

// includes ///////////////////////////////////////////////////////////////////////////////////////
#include<p18f4550.h>
#include"USBFunctions.h"

// chip config ////////////////////////////////////////////////////////////////////////////////////
					// clock options, see 18F4550 data sheet figure 2-1 "clock diagram" for explanation
#pragma config PLLDIV = 5				// 20 MHz external clock / PLL prescaler value of 5 = 4 MHz required input to PLL circuit
#pragma config CPUDIV = OSC1_PLL2		// non-PLL postscale / 1 OR PLL postscale / 2 for CPU clock speed, depending on FOSC setting below
#pragma config USBDIV = 2				// USB clock source = 96 MHz PLL source / 2, (full-speed USB mode)

						// if desired, could change this line to "FOSC = HS" & "oscillator postscaler" gate would be used 
						// (not the "PLL postscaler" gate), CPU speed would be 20MHz, USB circuitry would still receive 48Mhz clock
#pragma config FOSC = HSPLL_HS			// use high-speed external osc crystal, & use PLL postscaler gate to feed CPU (CPU speed = 48 MHz)

					// now the other less confusing options . . .
        #pragma config FCMEN    = OFF
        #pragma config IESO     = OFF
        #pragma config PWRT     = OFF
        #pragma config BOR      = ON
        #pragma config BORV     = 3
        #pragma config VREGEN   = ON      //USB Voltage Regulator
        #pragma config WDT      = OFF
        #pragma config WDTPS    = 32768
        #pragma config MCLRE    = ON
        #pragma config LPT1OSC  = OFF
        #pragma config PBADEN   = OFF
//      #pragma config CCP2MX   = ON
        #pragma config STVREN   = ON
        #pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
        #pragma config XINST    = OFF       // Extended Instruction Set
        #pragma config CP0      = OFF
        #pragma config CP1      = OFF
//      #pragma config CP2      = OFF
//      #pragma config CP3      = OFF
        #pragma config CPB      = OFF
//      #pragma config CPD      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
//      #pragma config WRTD     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
        #pragma config EBTRB    = OFF

// #defines ///////////////////////////////////////////////////////////////////////////////////////
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008			// these are necessary to accommodate the special linker file,
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018			// do not change these !!

#define START_BUTTON		PORTBbits.RB1			// input pins
#define STOP_BUTTON			PORTBbits.RB2

#define LED1	PORTDbits.RD1			// output pins
#define LED2	PORTDbits.RD2
#define LED3	PORTDbits.RD3

#define LED1_ON		0x01			// LED command states
#define LED1_OFF	0x00

#define LED2_ON		0x01
#define LED2_OFF	0x00

#define LED3_ON		0x01
#define LED3_OFF	0x00

#define SWITCH1_ON	0x01			// switch states
#define SWITCH1_OFF	0x00

#define SWITCH2_ON	0x01
#define SWITCH2_OFF	0x00

#define START_TIME 	0x01
#define STOP_TIME	0x02

// global variables ///////////////////////////////////////////////////////////////////////////////
extern BYTE g_USBDeviceState;
extern BYTE g_fromHostToDeviceBuffer[65];
extern BYTE g_fromDeviceToHostBuffer[65];

		// format for g_fromHostToDeviceBuffer[]
		// 
		// byte - purpose
		// -------------------------------------
		//	0 - do NOT use this for data, this is initialized to zero in USBstuff.c, leave as is (part of USB protocol)
		//	1 - LED1 state
		//	2 - LED2 state
		//	3 - LED3 state
		//	4 - unused
		//	5 - unused
		//	6 - unused
		//	7 - unused
		//	8 - usused
		//	9 - unused
		// 10 - unused
		// 11 - unused
		// 12 - unused
		// 13 - unused
		// 14 - usused
		// 15 - unused
		// 16 - unused
		// 17 - unused
		// 18 - unused
		// 19 - unused
		// 20 - usused
		// 21 - unused
		// 22 - unused
		// 23 - unused
		// 24 - unused
		// 25 - unused
		// 26 - usused
		// 27 - unused
		// 28 - unused
		// 29 - unused
		// 30 - unused
		// 31 - usused
		// 32 - unused
		// 33 - unused
		// 34 - unused
		// 35 - unused
		// 36 - unused
		// 37 - usused
		// 38 - unused
		// 39 - unused
		// 40 - unused
		// 41 - unused
		// 42 - unused
		// 43 - usused
		// 44 - unused
		// 45 - unused
		// 46 - unused
		// 47 - unused
		// 48 - unused
		// 49 - usused
		// 50 - unused
		// 51 - unused
		// 52 - unused
		// 53 - unused
		// 54 - unused
		// 55 - usused
		// 56 - unused
		// 57 - unused
		// 58 - unused
		// 59 - unused
		// 60 - unused
		// 61 - usused
		// 62 - unused
		// 63 - unused
		// 64 - unused
		
		// format for g_fromDeviceToHostBuffer[]
		// 
		// byte - purpose
		// -------------------------------------
		//	0 - do NOT use this for data, this is initialized to zero in USBstuff.c, leave as is (part of USB protocol)
		//	1 - SWITCH1 state
		//	2 - SWITCH2 state
		//	3 - unused
		//	4 - unused
		//	5 - unused
		//	6 - unused
		//	7 - unused
		//	8 - usused
		//	9 - unused
		// 10 - unused
		// 11 - unused
		// 12 - unused
		// 13 - unused
		// 14 - usused
		// 15 - unused
		// 16 - unused
		// 17 - unused
		// 18 - unused
		// 19 - unused
		// 20 - usused
		// 21 - unused
		// 22 - unused
		// 23 - unused
		// 24 - unused
		// 25 - unused
		// 26 - usused
		// 27 - unused
		// 28 - unused
		// 29 - unused
		// 30 - unused
		// 31 - usused
		// 32 - unused
		// 33 - unused
		// 34 - unused
		// 35 - unused
		// 36 - unused
		// 37 - usused
		// 38 - unused
		// 39 - unused
		// 40 - unused
		// 41 - unused
		// 42 - unused
		// 43 - usused
		// 44 - unused
		// 45 - unused
		// 46 - unused
		// 47 - unused
		// 48 - unused
		// 49 - usused
		// 50 - unused
		// 51 - unused
		// 52 - unused
		// 53 - unused
		// 54 - unused
		// 55 - usused
		// 56 - unused
		// 57 - unused
		// 58 - unused
		// 59 - unused
		// 60 - unused
		// 61 - usused
		// 62 - unused
		// 63 - unused
		// 64 - unused
		
// function prototypes ////////////////////////////////////////////////////////////////////////////
void highISR(void);							// interrupt prototypes
void remappedHighISR(void);					//
void yourHighPriorityISRCode(void);			//
											//
void lowISR(void);							//
void remappedLowISR(void);					//
void yourLowPriorityISRCode(void);			//
											//
extern void _startup(void);					//

void yourInit(void);
void yourTasks(void);

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void highISR(void) {
	_asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
void remappedHighISR(void) {
	_asm goto yourHighPriorityISRCode _endasm
}
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma interrupt yourHighPriorityISRCode
void yourHighPriorityISRCode(void) {
	// check which int flag is set
	// service int
	// clear flag
	// etc.
} // return will be a "retfie fast"
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma code LOW_INTERRUPT_VECTOR = 0x18
void lowISR(void) {
	_asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
void remappedLowISR(void) {
	_asm goto yourLowPriorityISRCode _endasm
}
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma interruptlow yourLowPriorityISRCode
void yourLowPriorityISRCode(void) {
	// check which int flag is set
	// service int
	// clear int flag
	// etc.
} // return will be a "retfie"
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma code REMAPPED_RESET_VECTOR = 0x1000
void _reset(void) {
	_asm goto _startup _endasm
}
#pragma code

///////////////////////////////////////////////////////////////////////////////////////////////////
void main(void) {
	USBInit();				// in USBFunctions.c
	yourInit();				// in this file
	while(1) {
		USBTasks();			// in USBFunctions.c
		yourTasks();		// in this file
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void yourInit(void) {
	// initialization code specific to your board goes here
	// (TRIS registers, ADCON registers, setting up interrupt / timer registers, etc.)
	
	ADCON1bits.PCFG3 = 1;			// set all AN pins to digital I/O
	ADCON1bits.PCFG2 = 1;			//
	ADCON1bits.PCFG1 = 1;			//
	ADCON1bits.PCFG0 = 1;			//
									
	TRISBbits.RB1 = 1;				// config input pins
	TRISBbits.RB2 = 1;
	
	TRISDbits.TRISD3 = 0;			// set RD3 to output
	TRISDbits.TRISD2 = 0;			// set RD2 to output
	TRISDbits.TRISD1 = 0;			// set RD1 to output
	
	LED3 = 0;						// init output pins to off
	LED2 = 0;
	LED1 = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void yourTasks(void) {

// your code here
//
// NOTES:
//
// The global variable g_USBDeviceState denotes the current state of the USB connection.  There are
// 6 states for g_USBDeviceState, each with matching #defines in USBStuff.h, as follows:
// DETACHED_STATE
// ATTACHED_STATE
// POWERED_STATE
// DEFAULT_STATE
// ADDRESS_STATE
// CONFIGURED_STATE
//
// Before doing anything with the USB buffer arrary, verify that:
//
// g_USBDeviceState == CONFIGURED_STATE
//
// There are 2 USB buffer arrays, g_fromHostToDeviceBuffer[65] and g_fromDeviceToHostBuffer[65].
// Hopefully it is clear from the names which goes which direction.  Each element is one byte.
// The first elements in each, g_fromHostToDeviceBuffer[0] and g_fromDeviceToHostBuffer[0], are
// the 'Report ID' and are part of the USB protocol, do not use these.  Start you data with
// g_fromHostToDeviceBuffer[1] and g_fromDeviceToHostBuffer[1].  This leaves 64 bytes of data
// for you to use.
//
// for USB I/O, call receiveViaUSB() and sendViaUSB()

	if(g_USBDeviceState == CONFIGURED_STATE) {
		
		receiveViaUSB();									// read into input buffer
		
															// process inputs here (check g_fromHostToDeviceBuffer[x])
		if(g_fromHostToDeviceBuffer[1] == LED1_ON) {
			LED1 = 1;
		} else if(g_fromHostToDeviceBuffer[1] == LED1_OFF) {
			LED1 = 0;
		} else {
			// should never get here
		}
		
		if(g_fromHostToDeviceBuffer[2] == LED2_ON) {
			LED2 = 1;
		} else if(g_fromHostToDeviceBuffer[2] == LED2_OFF) {
			LED2 = 0;
		} else {
			// should never get here
		}
		
		if(g_fromHostToDeviceBuffer[3] == LED3_ON) {
			LED3 = 1;
		} else if(g_fromHostToDeviceBuffer[3] == LED3_OFF) {
			LED3 = 0;
		} else {
			// should never get here
		}

		g_fromDeviceToHostBuffer[1] = (START_BUTTON<<1) | (STOP_BUTTON<<2);
		
		g_fromDeviceToHostBuffer[3] = 10;
		sendViaUSB();
	}		

}
