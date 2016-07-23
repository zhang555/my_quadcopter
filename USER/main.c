#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "includes.h"

#include "sysconfig.h"
#include "delay.h"
#include "sys.h"
#include "NRF_upload.h"

#include "usart.h"
#include "timer.h"
#include "myiic.h"
#include "MPU6050.h"		
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "control.h"
#include "math.h"
#include "24l01.h"
#include "RC.h"
#include <stdio.h>



S_FLOAT_PRY   Q_ANGLE,Pre_Q_ANGLE;			//四元数计算出的角度
S_FLOAT_XYZ   Q_GYRO;

S_INT16_RC 		RC_GET;

u8 armed=0;


u32 temp=0;

u8 tmp_buf[32]={0};	 

u8 NRF_payload_buf[16]={0};

	u8 i=0;




#define CONTROL_TASK_PRIO       			4 
#define CONTROL_STK_SIZE  		    		64
OS_STK CONTROL_TASK_STK[CONTROL_STK_SIZE];

void control_task(void *pdata)
{	 	
	OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		
	while(1)
	{
		OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
		
					mpu_dmp_get_data(&Q_ANGLE,&Q_GYRO);
		PIDPID();
		
		OS_EXIT_CRITICAL();	
		
		
		delay_ms(10);
	}
}



#define RCANL_TASK_PRIO       			6 
#define RCANL_STK_SIZE  					64
OS_STK RCANL_TASK_STK[RCANL_STK_SIZE];

void rcanl_task(void *pdata)
{	  
	OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		
	
	while(1)
	{
		
//			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);//允许CC1IE捕获中断	 
//			TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);//允许CC2IE捕获中断	
//			TIM_ITConfig(TIM4,TIM_IT_CC3,ENABLE);//允许CC3IE捕获中断	
//			TIM_ITConfig(TIM4,TIM_IT_CC4,ENABLE);//允许CC4IE捕获中断
//				
//			TIM_ITConfig(TIM5,TIM_IT_CC1,ENABLE);//允许CC1IE捕获中断	 
//			TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//允许CC2IE捕获中断	
		
//		delay_ms(1);

//			TIM_ITConfig(TIM4,TIM_IT_CC1,DISABLE);//
//			TIM_ITConfig(TIM4,TIM_IT_CC2,DISABLE);//
//			TIM_ITConfig(TIM4,TIM_IT_CC3,DISABLE);//
//			TIM_ITConfig(TIM4,TIM_IT_CC4,DISABLE);//
//				
//			TIM_ITConfig(TIM5,TIM_IT_CC1,DISABLE);//
//			TIM_ITConfig(TIM5,TIM_IT_CC2,DISABLE);//
		
			OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
		
		RC_anl(&RC_GET);		
		
			OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)

		delay_ms(10);
	}
}

#define UPLOAD_TASK_PRIO       			8 
#define UPLOAD_STK_SIZE  					64
OS_STK UPLOAD_TASK_STK[UPLOAD_STK_SIZE];

void upload_task(void *pdata)
{	  
	while(1)
	{
		Upload_01();
		Upload_02();
		Upload_03();
		Upload_06();
		Upload_10();
		
		printf("OSCPUUsage is %d \n",OSCPUUsage);
		
		delay_ms(20);
	}
}



#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
#define START_STK_SIZE  				64
OS_STK START_TASK_STK[START_STK_SIZE];


void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		
  
 			  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    

	OSTaskCreate(	control_task,
								(void *)0,
								(OS_STK*)&CONTROL_TASK_STK[CONTROL_STK_SIZE-1],
								CONTROL_TASK_PRIO);			
	
	
 	OSTaskCreate(	rcanl_task,
								(void *)0,
								(OS_STK*)&RCANL_TASK_STK[RCANL_STK_SIZE-1],
								RCANL_TASK_PRIO);
	
	OSTaskCreate(	upload_task,
								(void *)0,
								(OS_STK*)&UPLOAD_TASK_STK[UPLOAD_STK_SIZE-1],
								UPLOAD_TASK_PRIO);
								
	
	
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}



 int main(void)
 {
	delay_init();	     //延时初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	LED_Init();		  	 //初始化与LED连接的硬件接口
	 
	 uart_init(115200);
	 
	IIC_Init();
	MPU_Init();					//初始化MPU6050
	mpu_dmp_init();
 
	 TIM_RC_Init();

		SPI1_NRF24L01_Init();    	//初始化NRF24L01 

	SPI1_NRF24l01_Mode(TX2_MODE);
	 
	OSInit();   
	

	OSTaskCreate(	start_task,
								(void *)0,
								(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],
								START_TASK_PRIO);									



	OSStart();
								
 }








