#include <p18f4550.h>
#include "delays.h"
#include <adc.h>
#include "lcd1.h"
#include <usart.h>
#include <EEP.h>

        #pragma config PLLDIV   = 5         // (20 MHz crystal on PICDEM FS USB board)
		#pragma config CPUDIV   = OSC1_PLL2 
        #pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
        #pragma config FOSC     = HSPLL_HS
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

//#define LCD_DEBUG
		
#define COT		PORTD
#define HANG	PORTB

#define on	0
#define off	1
#define BAUD_RATE 312
#define MAXIMUM_DATA_SIZE 8
#define BASE_OF_LEN	4

#define OP_CODE_ONOFF_ONE_LED		0x01
#define	OP_CODE_GET_STAE_ALL_LED	0x02
#define	OP_CODE_GET_STAE_ONE_LED	0x03

typedef struct {
	char start_cmd;
	char lenOfData;
	char op_code;
	char data[MAXIMUM_DATA_SIZE+1];		// data + stopcmd
} dataFormat;

void InterruptHandlerHigh (void);

int addrMatrixValueDefault[8] = {
	0x00,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
};

char cot_value[8] = {
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	};

dataFormat bufferTodataFormat (char buffer[]){
	dataFormat data;
	int i;
	
	data.start_cmd = buffer[0];
	data.lenOfData = buffer[1];
	data.op_code = buffer[2];
	for (i=0; i<data.lenOfData; i++)
		data.data[i] = buffer[3+i];
	//data.data[i] = buffer[BASE_OF_LEN+data.lenOfData];
	
	//LCD_number(data.start_cmd); WriteDataToLCD(' ');
	//LCD_number(data.lenOfData); WriteDataToLCD(' ');
	//LCD_number(data.op_code);   WriteDataToLCD(' ');
	return data;
}
	
void getValueDefaultMatrix(){
	int i;
	for (i=0; i<8; i++){
		cot_value[i] = Read_b_eep(addrMatrixValueDefault[i]);
	}
}

void UART_Write(char data)
{
  TXSTAbits.TXEN = 1;
  while(!TXSTAbits.TRMT);
  TXREG = data;
}

char UART_TX_Empty()
{
  return TXSTAbits.TRMT;
}

void printf_uart(char *text)
{
  int i;
  for(i=0;text[i]!='\0';i++)
    UART_Write(text[i]);
}

void init_uart (){
	
	TRISCbits.RC6 = 0; 
	TRISCbits.RC7 = 1; 
	
	//step 1
	BAUDCONbits.BRG16 = 1;
	TXSTAbits.BRGH = 0;
	SPBRGH = (BAUD_RATE >> 8) & 0xFF;
	SPBRG  =  BAUD_RATE & 0xFF;
	
	//step 2
	TXSTAbits.SYNC = 0;
	RCSTAbits.SPEN = 1;
	
	//step 3
	BAUDCONbits.TXCKP = 0;
	
	//step 4
	PIE1bits.RCIE = 1;	// recevice interrupt enable
	PIE1bits.TXIE = 1;	// transmit interrupt enable
	
	//step 5 
	RCSTAbits.RX9 = 0;
	
	//step 6
	RCSTAbits.CREN = 1;
	
	//step 11
	RCONbits.IPEN = 1;
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	IPR1bits.RCIP = 1; // recevice interrupt high period
	IPR1bits.TXIP = 1; // transmit interrupt high period
	
	/*transmit setup*/
	TXSTAbits.TX9 = 0; // Selects 8-bit transmission	
}
	
void set_led(int led, int onoff)
{
	int hang, cot;
	hang = led/8;
	cot = led%8;
	
	switch(onoff){
		case on:
			cot_value[cot] = cot_value[cot] & (~(0x80>>hang));	// bit cot trong cot_value[hang] = 1
			break;
		case off:
			cot_value[cot] = cot_value[cot] | (0x80>>hang);		// bit cot trong cot_value[hang] = 0
			break;
		default:
			break;
	}
	
	Write_b_eep(addrMatrixValueDefault[cot], cot_value[cot]);
}
	
void main(void)
{
	int i;
	TRISD = 0x00;
	TRISB = 0x00;
	TRISC = 0x00;
	TRISA = 0x00;
	
	// configure USART
	InitLCD();
	init_uart ();
	getValueDefaultMatrix();
	WriteDataToLCD(' ');
	
 	while(1)
 	{
		UART_Write('m');
		for (i=0; i<8;){
			COT = (1<<i);
			HANG = cot_value[i];
			i++;
			delay(100);
		}
	}

}//end main

#pragma code InterruptVectorHigh = 0x08

void InterruptVectorHigh (void)
{
	_asm
		goto InterruptHandlerHigh //jump to interrupt routine
	_endasm
}

#pragma code
#pragma interrupt InterruptHandlerHigh
int size = 0;
char buffer[];
dataFormat dataRecevice;
dataFormat dataTranfer;

void handlerBuffer (char bufferData[], int sizeBuffer){
	int led;
	int onoff;
	
	dataRecevice = bufferTodataFormat (bufferData);
	
	dataTranfer.start_cmd = dataRecevice.start_cmd;
	
#ifdef LCD_DEBUG
	LCD_number(dataRecevice.start_cmd); WriteDataToLCD(' ');
	LCD_number(dataRecevice.lenOfData); WriteDataToLCD(' ');
	LCD_number(dataRecevice.op_code);   WriteDataToLCD(' ');
#endif
	
	switch(dataRecevice.op_code){
		case OP_CODE_ONOFF_ONE_LED:
			led = dataRecevice.data[0];
			if (dataRecevice.data[1]==0)
				onoff = off;
			else{
				if (dataRecevice.data[1]==1)
					onoff = on;
			}
			set_led(led, onoff);
			break;
		case OP_CODE_GET_STAE_ALL_LED:
			break;
		case OP_CODE_GET_STAE_ONE_LED:
			break;
		default:
			break;
	}
}

void
InterruptHandlerHigh (){
	char temp;
	
	if (PIR1bits.RCIF){
		if (size == 0)
			ClearLCDScreen();
		
		//PORTCbits.RC5 = 1 - PORTCbits.RC5;
		buffer[size] = RCREG;
		//LCD_number(buffer[size]);
		size++;
		if (buffer[0]==0x4B){
			if (buffer[size-1] == 0x7E){
				handlerBuffer(buffer, size);
				size = 0;
			}
		} else {
			size = 0;
		}
		UART_Write('a');
		//UART_Write('b');
		//UART_Write('c');
		//UART_Write('\n');
		printf_uart("hoang van cuong");
	}
	
	if (PIR1bits.TXIF){
		while(!TXSTAbits.TRMT);
		TXSTAbits.TXEN = 0;
		PORTCbits.RC5 = 1 - PORTCbits.RC5;
	}
	
}

