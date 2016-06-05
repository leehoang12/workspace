	// USBDemoBoard.c

// includes ///////////////////////////////////////////////////////////////////////////////////////
#include<p18f4550.h>
#include"USBFunctions.h"
#include "mpu6050.h"
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

#define MPU6050_ADDRESS 0x68
#define MPU6050_RA_WHO_AM_I 0x75

// global variables ///////////////////////////////////////////////////////////////////////////////
extern BYTE g_USBDeviceState;
extern BYTE g_fromHostToDeviceBuffer[65];
extern BYTE g_fromDeviceToHostBuffer[65];

 unsigned char verificacion = 0x0;
	
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
		LED3 = 1;						// init output pins to off
		LED2 = 1;
		LED1 = 1;
		USBTasks();			// in USBFunctions.c
		yourTasks();		// in this file
	}
}

 unsigned int LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length)  
 {     
	unsigned int error = 0;     
	IdleI2C();                                  //wait for bus Idle     
	StartI2C();                                 //Generate Start Condition     
	WriteI2C(ControlByte | 0x00);               //Write Control Byte     
	IdleI2C();                                  //wait for bus Idle     
	WriteI2C(Address);                          //Write start address     
	IdleI2C();                                  //wait for bus Idle  
	RestartI2C();                               //Generate restart condition     
	WriteI2C(ControlByte | 0x01);               //Write control byte for read     
	IdleI2C();                                  //wait for bus Idle  
	error = getsI2C(Data, Length);                      //read Length number of bytes     
	NotAckI2C();                                //Send Not Ack     
	StopI2C();                                  //Generate Stop     
	return error;  
 }  
   

///////////////////////////////////////////////////////////////////////////////////////////////////
void yourInit(void) {
	
	TRISBbits.RB0 = 1;				// config input pin SDA
	TRISBbits.RB1 = 1;				// config input pin SCL	
	
	TRISDbits.TRISD3 = 0;			// set RD3 to output
	TRISDbits.TRISD2 = 0;			// set RD2 to output
	TRISDbits.TRISD1 = 0;			// set RD1 to output
	
	LED3 = 0;						// init output pins to off
	LED2 = 0;
	LED1 = 0;
	
	OpenI2C(MASTER, SLEW_OFF);		
	//Mpu6050_Init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void yourTasks(void) {
	int x;
	x = LED3;
	//LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I, &verificacion, 1);
	LED3 = 1-x;
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

		
		g_fromDeviceToHostBuffer[1] = verificacion;
		
		g_fromDeviceToHostBuffer[3] = 10;
		sendViaUSB();
	}		

}
