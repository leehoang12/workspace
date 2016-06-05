#ifndef __MPU6050_H
#define __MPU6050_H
#include "i2c.h"

#define   SMPLRT_DIV      0x19   //0x07(125Hz)
#define   CONFIG         0x1A   //0x06(5Hz)
#define   GYRO_CONFIG      0x1B   //0x18(²»×Ô¼ì£¬2000deg/s)
#define   ACCEL_CONFIG   0x1C   //0x01(²»×Ô¼ì£¬2G£¬5Hz)
#define   ACCEL_XOUT_H   0x3B
#define   ACCEL_XOUT_L   0x3C
#define   ACCEL_YOUT_H   0x3D
#define   ACCEL_YOUT_L   0x3E
#define   ACCEL_ZOUT_H   0x3F
#define   ACCEL_ZOUT_L   0x40
#define   TEMP_OUT_H      0x41
#define   TEMP_OUT_L      0x42
#define   GYRO_XOUT_H      0x43
#define   GYRO_XOUT_L      0x44   
#define   GYRO_YOUT_H      0x45
#define   GYRO_YOUT_L      0x46
#define   GYRO_ZOUT_H      0x47
#define   GYRO_ZOUT_L      0x48
#define   PWR_MGMT_1      0x6B   //
#define   WHO_AM_I         0x75   //

#define MPU6505_ADDR_WRITE 0xD0  //dia chi cua cam bien mpu6050
#define MPU6505_ADDR_READ 0xD1  //dia chi cua cam bien mpu6050

void convert (unsigned char *s,int temp_data) ;       
void Mpu6050_Init();
int GetData(unsigned char address);
unsigned char Mpu6050_Read(unsigned char address);
void Mpu6050_Write(unsigned char address,unsigned char Data);
#endif
