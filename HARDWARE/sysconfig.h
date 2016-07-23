#ifndef _SYSCONFIG_H_
#define _SYSCONFIG_H_

#include "sys.h"
#define Gyro_deg 	0.0609756f			//角速度单位转化成角度每秒

#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)      ) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )
	
extern float OUT_P,OUT_I,OUT_D;
extern float IN_P,IN_I,IN_D;

typedef struct{
				int16_t X;
				int16_t Y;
				int16_t Z;}S_INT16_XYZ;
typedef struct{
				float X;
				float Y;
				float Z;}S_FLOAT_XYZ;
typedef struct{
				float pit;
				float rol;
				float yaw;}S_FLOAT_PRY;
typedef struct{
				s16 THROTTLE;
				s16 ROLL;
				s16 PITCH;
				s16 YAW;
				s16 AUX1;
				s16 AUX2;
				s16 AUX3;
				s16 AUX4;
				s16 AUX5;
				s16 AUX6;}S_INT16_RC;



extern S_INT16_XYZ		GYRO_OFFSET,ACC_OFFSET;			//零漂




extern int16_t THROTTLE;



//#define q30  1073741824.0f
//extern float q0,q1,q2,q3;
//extern char num[50];
//extern unsigned long sensor_timestamp;
//extern short gyro[3], accel[3], sensors;
//extern unsigned char more;
//extern long quat[4];
//extern signed char gyro_orientation[9] ;

#endif
