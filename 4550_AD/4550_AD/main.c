#include <p18f4550.h>
#include "delays.h"
#include <adc.h>
#include <lcd.h>

void main(void)
{

	int data;
	TRISA = 0xFF;		
	
	InitLCD();

	OpenADC( ADC_FOSC_16 & 
	ADC_RIGHT_JUST &
	ADC_16_TAD ,
	ADC_VREFPLUS_EXT &
	ADC_VREFMINUS_EXT &
	ADC_CH0 &
	ADC_INT_OFF ,
	0x0d);
	
 while(1)
 {
	WriteCommandToLCD(0x01);
    WriteCommandToLCD(0x06);
    WriteCommandToLCD(0x06);


	SetChanADC(ADC_CH0);	//AN 0
	ConvertADC();			//Start ADC
	while(BusyADC());		//Wait ADC
	data = ReadADC();		//Get A/D data
	LCD_number(data);
	//delay(100);
	WriteDataToLCD(' ');	

	SetChanADC(ADC_CH1);	//AN 1
	ConvertADC();			//Start ADC
	while(BusyADC());		//Wait ADC
	data = ReadADC();		//Get A/D data
	LCD_number(data);
	}

}//end main

//configulation***********************************************
        #pragma config PLLDIV   = 5         // (20 MHz crystal on PICDEM FS USB board)
        #pragma config CPUDIV   = OSC1_PLL2   
        #pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
        #pragma config FOSC     = HS//YTS PLL_HS
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
/** EOF main.c ***************************************************************/
