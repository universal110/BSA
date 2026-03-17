
#ifndef SENSOR_H
#define SENSOR_H


#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#define sensor_ADDR      0x68

#define sensor_PWR_MGMT  0x6B


#define sensor_ACCEL_XOUT_H 0x3B  
#define sensor_ACCEL_XOUT_L 0x3C  
#define sensor_ACCEL_YOUT_H 0x3D 
#define sensor_ACCEL_YOUT_L 0x3E 

#define sensor_GYRO_XOUT_H 0x43 
#define sensor_GYRO_XOUT_L 0x44  
#define sensor_GYRO_YOUT_H 0x45
#define sensor_GYRO_YOUT_L 0x46  

int sensorInit(void);
float sensorWristTurn(void); // sensor roll
float sensorWristBend(void); // sensor pitch
float sensorGetDeviation(float sidewaysTilt, float wristBend);
float sensorGetMagnitude(void);


#endif