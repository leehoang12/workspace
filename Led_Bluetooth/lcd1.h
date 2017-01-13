#include <p18cxxx.h>

#define LCD_E	LATCbits.LATC0
#define LCD_RW	LATCbits.LATC1
#define LCD_RS	LATCbits.LATC2
#define LCD_D4	LATAbits.LATA0
#define LCD_D5	LATAbits.LATA1
#define LCD_D6	LATAbits.LATA2
#define LCD_D7	LATAbits.LATA3

#define LCD_E_Dir	TRISDbits.TRISD0
#define LCD_RW_Dir	TRISDbits.TRISD1
#define LCD_RS_Dir	TRISDbits.TRISD2
#define LCD_D4_Dir	TRISDbits.TRISD4
#define LCD_D5_Dir	TRISDbits.TRISD5
#define LCD_D6_Dir	TRISDbits.TRISD6
#define LCD_D7_Dir	TRISDbits.TRISD7

void delay (int x);
void latch (void);
void LCDSend4bit (unsigned char Data);
void WriteCommandToLCD (unsigned char Command);
void WriteDataToLCD (char LCDChar);
void InitLCD (void);
void WriteStringToLCD (const char *s);
void ClearLCDScreen (void);
void LCD_number (unsigned int d);


void delay (int x){
	while (x--);
}

void latch (void){
	LCD_E = 1;
	delay(100);
	LCD_E = 0;
	delay(100);
}

void LCDSend4bit (unsigned char Data){
	LCD_D4 = (Data >> 0) & 0x01;
	LCD_D5 = (Data >> 1) & 0x01;
	LCD_D6 = (Data >> 2) & 0x01;
	LCD_D7 = (Data >> 3) & 0x01;
	latch();
}

void WriteCommandToLCD (unsigned char Command){
	LCD_RS = 0;
	LCDSend4bit(Command >> 4);
	LCDSend4bit (Command);
}

void WriteDataToLCD (char LCDChar){
	LCD_RS = 1;
	LCDSend4bit (LCDChar >> 4);
	LCDSend4bit (LCDChar);
}

void InitLCD (void){
	LCD_E_Dir = 0;
	LCD_RS_Dir = 0;
	LCD_RW_Dir = 0;
	LCD_D4_Dir = 0;
	LCD_D5_Dir = 0;
	LCD_D6_Dir = 0;
	LCD_D7_Dir = 0;
	
	LCD_RW = 0;
	WriteCommandToLCD(0x32);
	delay(100);
	WriteCommandToLCD(0x28);
	WriteCommandToLCD(0x0C);
	WriteCommandToLCD(0x06);
	WriteCommandToLCD(0x01);
}

void WriteStringToLCD(const char *s){
	while (*s)
		WriteDataToLCD(*s++);
}

void ClearLCDScreen(void){
	WriteCommandToLCD(0x01);
	delay(100);
}

void LCD_number (unsigned int d){
	unsigned char a1, a2, a3, a4, a5;
	
	a1 = d%10;
	d = d/10;
	a2 = d%10;
	d = d/10;
	a3 = d%10;
	
	WriteDataToLCD(48+a3);
	WriteDataToLCD(48+a2);
	WriteDataToLCD(48+a1);
}










