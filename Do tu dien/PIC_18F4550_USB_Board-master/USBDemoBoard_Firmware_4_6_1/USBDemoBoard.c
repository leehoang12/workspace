// USBDemoBoard.c

// includes ///////////////////////////////////////////////////////////////////////////////////////
#include<p18f4550.h>
#include"USBFunctions.h"
#include <adc.h>

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

#define VDC_150_STATE		PORTBbits.RB3		// config VDC state
#define VDC_600_STATE		PORTBbits.RB4
#define VDC_990_STATE		PORTBbits.RB5
#define VDC_1120_STATE		PORTBbits.RB6
#define VDC_150				(VDC_150_STATE && !VDC_600_STATE && !VDC_990_STATE && !VDC_1120_STATE)
#define VDC_600				(!VDC_150_STATE && VDC_600_STATE && !VDC_990_STATE && !VDC_1120_STATE)
#define VDC_990				(!VDC_150_STATE && !VDC_600_STATE && VDC_990_STATE && !VDC_1120_STATE)
#define VDC_1120			(!VDC_150_STATE && !VDC_600_STATE && !VDC_990_STATE && VDC_1120_STATE)

#define CHANNEL_CAP1	ADC_CH0
#define CHANNEL_CAP2	ADC_CH1


// global variables ///////////////////////////////////////////////////////////////////////////////
extern BYTE g_USBDeviceState;
extern BYTE g_fromHostToDeviceBuffer[65];
extern BYTE g_fromDeviceToHostBuffer[65];
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
	LED3 = ~LED3;
}
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

int detect_vdc(){
	if (VDC_150)
		return 150;
	else{
		if (VDC_600)
			return 600;
		else {
			if (VDC_990)
				return 990;
			else{
				if (VDC_1120)
					return 1120;
			}
		}
	}
	
	return -1;
}

void yourInit(void) {
	// initialization code specific to your board goes here
	// (TRIS registers, ADCON registers, setting up interrupt / timer registers, etc.)
	
	ADCON1bits.PCFG3 = 1;			// set all AN pins to digital I/O
	ADCON1bits.PCFG2 = 1;			//
	ADCON1bits.PCFG1 = 1;			//
	ADCON1bits.PCFG0 = 1;			//
									
	TRISBbits.RB1 = 1;				// Start button
	TRISBbits.RB2 = 1;				// Stop button

	TRISBbits.RB3 = 1;
	TRISBbits.RB4 = 1;
	TRISBbits.RB5 = 1;
	TRISBbits.RB6 = 1;
	
	TRISDbits.TRISD3 = 0;			// set RD3 to output
	TRISDbits.TRISD2 = 0;			// set RD2 to output
	TRISDbits.TRISD1 = 0;			// set RD1 to output
	
	LED3 = 0;						// init output pins to off
	LED2 = 0;
	LED1 = 0;


   /*ADC int */
	TRISA = 0xFF;		
	PIR1bits.ADIF = 0; // clear ADIF (ADC flag interrup)
	PIE1bits.ADIE = 1; // Enable interrup ADC
	INTCONbits.GIE = 1; // enable all interrupt

	OpenADC( ADC_FOSC_16 & ADC_RIGHT_JUST & ADC_16_TAD ,
	ADC_VREFPLUS_EXT & ADC_VREFMINUS_EXT & ADC_CH0 & ADC_INT_OFF , 0x0d);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void yourTasks(void) {
	int ret;

	if(g_USBDeviceState == CONFIGURED_STATE) {
		
		receiveViaUSB();

		g_fromDeviceToHostBuffer[1] = (START_BUTTON<<1) | (STOP_BUTTON<<2);		// Start/Stop buuton
		ret = detect_vdc();
		if (ret<0){
			g_fromDeviceToHostBuffer[2] = 0;
			g_fromDeviceToHostBuffer[3] = 0;
		} else {
			g_fromDeviceToHostBuffer[2] = (ret >> 8) & 0xFF;		// 8 bits hight of VDC
			g_fromDeviceToHostBuffer[3] =  ret & 0xFF;				// 8 buts low of VDC
		}

		SetChanADC(CHANNEL_CAP1);					// read data cap 1
		ConvertADC();
		while(BusyADC());							//Wait ADC
			ret = ReadADC();						//Get A/D data	
		g_fromDeviceToHostBuffer[4] = (ret >> 8) & 0xFF;		// 8 bits hight of value CAP 1
		g_fromDeviceToHostBuffer[5] =  ret & 0xFF;				// 8 buts low of value CAP 1
		
		SetChanADC(CHANNEL_CAP2);					// read data cap 1
		ConvertADC();
		while(BusyADC());							//Wait ADC
			ret = ReadADC();						//Get A/D data
		g_fromDeviceToHostBuffer[6] = (ret >> 8) & 0xFF;		// 8 bits hight of value CAP 1
		g_fromDeviceToHostBuffer[7] =  ret & 0xFF;				// 8 buts low of value CAP 2	

		sendViaUSB();
	}
}
