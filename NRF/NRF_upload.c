#include "sysconfig.h"
#include "sys.h"
#include "usart.h"	  
#include "24l01.h"


#define USART_SEND_DATA
//#define NRF_SEND_DATA

extern S_FLOAT_PRY 		Q_ANGLE,Pre_Q_ANGLE;					
extern S_INT16_RC 		RC_GET;
extern S_FLOAT_XYZ   Q_GYRO;
extern u8 armed;			
extern u16 speed0,speed1,speed2,speed3;
extern float IN_P,IN_I,IN_D;

//上传 Roll Pitch Yaw 数据  高度数据
void Upload_01()
{
	u8 send_buf[32]={0};
	u8 len=17; //长度  2+1+1+12+1
	u8 i;
	u8 sum = 0;

	send_buf[0]=0xAA;
	send_buf[1]=0xAA;
	send_buf[2]=0x01;
	send_buf[3]=len-5;
	send_buf[4]=(((int)(Q_ANGLE.rol*100))>>8)&0XFF;  //高8位
	send_buf[5]=((int)(Q_ANGLE.rol*100))&0XFF;  //低8位
	send_buf[6]=(((int)(Q_ANGLE.pit*100))>>8)&0XFF;
	send_buf[7]=((int)(Q_ANGLE.pit*100))&0XFF;
	send_buf[8]=(((int)(Q_ANGLE.yaw*100))>>8)&0XFF;
	send_buf[9]=((int)(Q_ANGLE.yaw*100))&0XFF;
	
//	send_buf[10]=BYTE3(*altitude);
//	send_buf[11]=BYTE2(*altitude);
//	send_buf[12]=BYTE1(*altitude);
//	send_buf[13]=BYTE0(*altitude);
	
//	send_buf[14]=0;
		send_buf[15]=armed;
//	send_buf[16]=0;
//	send_buf[17]=0;
//	send_buf[18]=0;
//	send_buf[19]=0;


	for(i=0;i<len-1;i++)
		sum += send_buf[i];
	send_buf[16] = sum;
	
#ifdef USART_SEND_DATA

	for(i=0;i<31;i++)
	{
		usart1_send_char(send_buf[i]);
	}
			
#endif
			
#ifdef NRF_SEND_DATA
	SPI1_NRF24L01_TxPacket(send_buf); 
#endif	
}
//上传三轴加速度 角速度
void Upload_02(void)
{
      u8 len=27; //长度  2+1+1+22+1

	u8 send_buf[32]={0};
	u8 i;
	
	u8 sum = 0;
         
	send_buf[0]=0xAA;
	send_buf[1]=0xAA;
	send_buf[2]=0x02;
	send_buf[3]=len-5;
        
	//三轴加速度
//	send_buf[4]=(left_count>>8)&0XFF;  //高8位
//	send_buf[5]=left_count&0XFF; //低8位

//        
//	send_buf[6]=(right_count>>8)&0XFF;
//	send_buf[7]=right_count&0XFF;
//        
//	send_buf[8]=(0>>8)&0XFF;
//	send_buf[9]=0&0XFF;

	//三轴角速度
	send_buf[10]=((s16)(Q_GYRO.X*1)>>8)&0XFF;
	send_buf[11]=(s16)(Q_GYRO.X*1)&0XFF;
	
	send_buf[12]=((s16)(Q_GYRO.Y*1)>>8)&0XFF;
	send_buf[13]=(s16)(Q_GYRO.Y*1)&0XFF;
        
	send_buf[14]=0;
	send_buf[15]=0;	

// 三轴磁场
//	send_buf[16]=0;
//	send_buf[17]=0;
//        
//	send_buf[18]=0;
//	send_buf[19]=0;	
//        
//	send_buf[20]=0;
//	send_buf[21]=0;	        
        
		//高度
//	send_buf[22]=0;
//	send_buf[23]=0;	        
//	send_buf[24]=0;
//	send_buf[25]=0;	        
        
        
	for(i=0;i<len-1;i++)
		sum += send_buf[i];
	send_buf[26] = sum;


#ifdef USART_SEND_DATA

	for(i=0;i<31;i++)
	{
		usart1_send_char(send_buf[i]);
	}
			
#endif
			
#ifdef NRF_SEND_DATA
	SPI1_NRF24L01_TxPacket(send_buf); 
#endif	

}

//上传遥控数据
void Upload_03(void)
{
	   u8 len=25; //长度  2+1+1+20+1
	u8 send_buf[32]={0};
	u8 i;
	u8 sum = 0;

	send_buf[0]=0xAA;
	send_buf[1]=0xAA;
	send_buf[2]=0x03;
	send_buf[3]=len-5;
	send_buf[4]=((RC_GET.THROTTLE)>>8)&0XFF;  //高8位
	send_buf[5]=(RC_GET.THROTTLE)&0XFF;  //低8位
	send_buf[6]=((RC_GET.YAW)>>8)&0XFF;
	send_buf[7]=(RC_GET.YAW)&0XFF;
	send_buf[8]=((RC_GET.ROLL)>>8)&0XFF;
	send_buf[9]=(RC_GET.ROLL)&0XFF;
	send_buf[10]=((RC_GET.PITCH)>>8)&0XFF;
	send_buf[11]=(RC_GET.PITCH)&0XFF;
	send_buf[12]=((RC_GET.AUX1)>>8)&0XFF;
	send_buf[13]=(RC_GET.AUX1)&0XFF;
	send_buf[14]=((RC_GET.AUX2)>>8)&0XFF;
	send_buf[15]=(RC_GET.AUX2)&0XFF;
	send_buf[16]=((RC_GET.AUX3)>>8)&0XFF;
	send_buf[17]=(RC_GET.AUX3)&0XFF;
	send_buf[18]=((RC_GET.AUX4)>>8)&0XFF;
	send_buf[19]=(RC_GET.AUX4)&0XFF;
	send_buf[20]=((RC_GET.AUX5)>>8)&0XFF;
	send_buf[21]=(RC_GET.AUX5)&0XFF;
	send_buf[22]=((RC_GET.AUX6)>>8)&0XFF;
	send_buf[23]=(RC_GET.AUX6)&0XFF;
	
	
	for(i=0;i<len-1;i++)
		sum += send_buf[i];
	send_buf[len-1] = sum;

#ifdef USART_SEND_DATA

	for(i=0;i<31;i++)
	{
		usart1_send_char(send_buf[i]);
	}
			
#endif
			
#ifdef NRF_SEND_DATA
	SPI1_NRF24L01_TxPacket(send_buf); 
#endif	
}
//上传电机PWM
void Upload_06(void) 
{
	u8 len=21; //长度  2+1+1+16+1
	u8 send_buf[32]={0};
	u8 i;
	u8 sum = 0;
	
	send_buf[0]=0xAA;
	send_buf[1]=0xAA;
	send_buf[2]=0x06;
	send_buf[3]=len-5;
	         
	send_buf[4]=(speed0>>8)&0XFF;  //高8位
	send_buf[5]=speed0&0XFF;  //低8位
	send_buf[6]=(speed1>>8)&0XFF;
	send_buf[7]=speed1&0XFF;	
	send_buf[8]=(speed2>>8)&0XFF;
	send_buf[9]=speed2&0XFF;
	send_buf[10]=(speed3>>8)&0XFF;
	send_buf[11]=speed3&0XFF;
//	send_buf[12]=0;
//	send_buf[13]=0;
//	send_buf[14]=0;
//	send_buf[15]=0;
//	send_buf[16]=0;
//	send_buf[17]=0;
//	send_buf[18]=0;
//	send_buf[19]=0;

	for(i=0;i<len-1;i++)
		sum += send_buf[i];
	send_buf[len-1] = sum;

#ifdef USART_SEND_DATA

	for(i=0;i<31;i++)
	{
		usart1_send_char(send_buf[i]);
	}
			
#endif
			
#ifdef NRF_SEND_DATA
	SPI1_NRF24L01_TxPacket(send_buf); 
#endif	
}

//上传PID参数
void Upload_10(void) 
{
	u8 len=23; //长度  2+1+1+18+1
	u8 send_buf[32]={0};
	u8 i;
	u8 sum = 0;
	
	send_buf[0]=0xAA;
	send_buf[1]=0xAA;
	send_buf[2]=0x10;
	send_buf[3]=len-5;

	send_buf[4]=(u16)IN_P>>8 &0XFF;  //高8位
	send_buf[5]=(u16)IN_P&0XFF;  //低8位
	send_buf[6]=(u16)IN_I>>8 &0XFF;
	send_buf[7]=(u16)IN_I&0XFF;	
	send_buf[8]=(u16)IN_D>>8 &0XFF;  //高8位
	send_buf[9]=(u16)IN_D&0XFF;  //低8位
	send_buf[10]=0;	
	send_buf[11]=0;
//	send_buf[12]=0;
//	send_buf[13]=0;
//	send_buf[14]=0;
//	send_buf[15]=0;
//	send_buf[16]=0;
//	send_buf[17]=0;
//	send_buf[18]=0;
//	send_buf[19]=0;

	for(i=0;i<len-1;i++)
		sum += send_buf[i];
	send_buf[len-1] = sum;

#ifdef USART_SEND_DATA

	for(i=0;i<31;i++)
	{
		usart1_send_char(send_buf[i]);
	}
			
#endif
			
#ifdef NRF_SEND_DATA
	SPI1_NRF24L01_TxPacket(send_buf); 
#endif	
}

//
//void Data_Receive_Anl(u8 *data_buf,u8 num)
//{
//	u8 i;
//	vs16 rc_value_temp;
//	u8 sum = 0;
//	
//	for(i=0;i<(num-1);i++)
//		sum += *(data_buf+i);
//	if(!(sum==*(data_buf+num-1)))		return;		//判断sum
//	if(!(*(data_buf)==0xAA && *(data_buf+1)==0xAF))		return;		//判断帧头
///////////////////////////////////////////////////////////////////////////////////////
//	if(*(data_buf+2)==0X03)
//	{
////		THROTTLE =((*(data_buf+4)<<8)|*(data_buf+5));			
//	}
//		
//	if(*(data_buf+2)==0X10)								//PID1
//	{
////			armed = (*(data_buf+4)<<8)|*(data_buf+5);
//			armed = (*(data_buf+6)<<8)|*(data_buf+7);	
//			THROTTLE = (*(data_buf+8)<<8)|*(data_buf+9);
//		
//			IN_P = (float)((vs16)(*(data_buf+10)<<8)|*(data_buf+11))/100;
//			IN_I = (float)((vs16)(*(data_buf+12)<<8)|*(data_buf+13))/100;
//			IN_D = (float)((vs16)(*(data_buf+14)<<8)|*(data_buf+15))/100;
//		
//		
//		
//			OUT_P = (float)((vs16)(*(data_buf+16)<<8)|*(data_buf+17))/100;
////			OUT_I = (float)((vs16)(*(data_buf+18)<<8)|*(data_buf+19))/100;
////			OUT_D = (float)((vs16)(*(data_buf+20)<<8)|*(data_buf+21))/100;
//		
//		
//	}
//	
//}




