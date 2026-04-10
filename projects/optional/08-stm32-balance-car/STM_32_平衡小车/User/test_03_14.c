#include "stm32f10x.h"                  // Device header
#include "Delay.h"  
#include "LED.h"
#include "Key.h"
#include "OLED.h"
#include "MPU6050.h"
#include "inv_mpu.h"
#include "Time.h"

float Kp = 1,Ki = 1,Kd = 1;

float VKp = 1,VKi = 1;


//u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz);
//u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az);

int main(void)
{
	OLED_Init();
	MPU6050_Init();
	MPU6050_DMP_Init();
	Time_Init();
	while(1)
	{
		
//		//╤ах║вкл╛пео╒
//		MPU6050_DMP_Get_Data(&pitch,&roll,&yaw);
//		MPU_Get_Gyroscope(&gx,&gy,&gz);
//		MPU_Get_Accelerometer(&ax,&ay,&az);
		
		
//		OLED_ShowSignedNum(1,1,pitch,3);
//		OLED_ShowSignedNum(2,1,roll,3);
//		OLED_ShowSignedNum(3,1,yaw,3);
		
		//MPU6050_GetData(&Acc_X,&Acc_Y,&Acc_Z,&Gyro_X,&Gyro_Y,&Gyro_Z);
		
		
	}
}
