#include "mpu6050.h"

void convert (unsigned char *s,int temp_data)
{
   if(temp_data<0)               //xax dinh dau - or + cua gia tri
   {
      temp_data=-temp_data;
      *s='-';
   }
   else *s=' ';
   *++s =temp_data/100+0x30;    //lay so hang tram
   temp_data=temp_data%100;     
   *++s =temp_data/10+0x30;    // lay so hang chuc
   temp_data=temp_data%10;      
   *++s =temp_data+0x30;      // lay so hang don vi 
}


void Mpu6050_Init()
{
   Mpu6050_Write(PWR_MGMT_1, 0x00);   
   Mpu6050_Write(SMPLRT_DIV, 0x07);
   Mpu6050_Write(CONFIG, 0x06);
   Mpu6050_Write(GYRO_CONFIG, 0x18);  
   Mpu6050_Write(ACCEL_CONFIG, 0x01); //old: 0x01
}

int GetData(unsigned char address)
{
   char H,L;
   H=Mpu6050_Read(address);
   L=Mpu6050_Read(address+1);
   return (H<<8)+L;   
}

unsigned char Mpu6050_Read(unsigned char reg) {
	
	unsigned char data;
	
	StartI2C();
	WriteI2C(MPU6505_ADDR_WRITE);
	AckI2C();
	WriteI2C(reg);
	AckI2C();
	
	StartI2C();
	WriteI2C(MPU6505_ADDR_READ);
	AckI2C();
	data = ReadI2C();
	AckI2C();
	StopI2C();
	
	return data;
}

void Mpu6050_Write(unsigned char reg,unsigned char data){
	StartI2C();
	WriteI2C(MPU6505_ADDR_WRITE);
	AckI2C();
	WriteI2C(reg);
	AckI2C();
	WriteI2C(data);
	StopI2C();
}