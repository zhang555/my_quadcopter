
#include "RC.h"
#include "sysconfig.h"
#include "sys.h"
#include "includes.h"					//ucos 使用	  

u16 RC_CH[6]={0};
extern u8 armed;
extern float IN_P;
//RC_CH[0]: 1224 1984     0 760   
/*		
						 单片机通道	  单片机引脚		接收机编号		遥控器位置			功能			范围							关闭值								运算后范围
	RC_CH[0]		TIM4_CH1			PB6						3				左拨杆 上下		油门大小		1890~1152				关闭后为1018					  40~416
	RC_CH[1]		TIM4_CH2			PB7						4				左拨杆 左右							1060~1520~1890		1520								-10~10
	RC_CH[2]		TIM4_CH3			PB8						5				右上 开关								开2030 关960			1520									0~1
	RC_CH[3]		TIM4_CH4			PB9						6				左上 旋钮			P值大小		1008~2024					1520								

	RC_CH[4]		TIM5_CH1			PA0						1				右拨杆 左右		横滚			1000~1416~1760												-10~10
	RC_CH[5]		TIM5_CH2			PA1						2				右拨杆 上下		俯仰			1200~1632~2000												-10~10
*/
u8 armed_flag=0;

void RC_anl(S_INT16_RC *rc_get)
{
	if(RC_CH[0] < 1152)
	{
		armed=0;
		rc_get->THROTTLE = 0;
		rc_get->YAW			 = 0;
		rc_get->ROLL		 = 0;
		rc_get->PITCH		 = 0;
	}
	else
	{

		rc_get->THROTTLE = (1984 - RC_CH[0]) / 2.0 ; //
		rc_get->YAW			 =   RC_CH[1];	
		
		rc_get->ROLL		 =  (RC_CH[4]-1416)/30;
		if(rc_get->ROLL > 10 )
			rc_get->ROLL= 10;
		if(rc_get->ROLL < -10 )
			rc_get->ROLL= -10;		

		rc_get->PITCH		 =   (RC_CH[5]-1632)/30;
		if(rc_get->PITCH > 10 )
			rc_get->PITCH= 10;
		if(rc_get->PITCH < -10 )
			rc_get->PITCH= -10;			
		
		
		rc_get->AUX1		 =  RC_CH[2];  //右上开关
		rc_get->AUX2		 =  armed*500;			
		
		rc_get->AUX3		 =  RC_CH[3];				//左上旋钮
		IN_P = ((float)RC_CH[3] - 1000) / 100.0;
		
//		rc_get->AUX4		 =  armed*500;	
//		rc_get->AUX5		 =  armed*500;	
//		rc_get->AUX6		 =  armed*500;			


		
		if(armed_flag==1 && RC_CH[2] > 1500)
		{
				armed=1;
			armed_flag=0;
		}
		
		if(RC_CH[2] < 1500)
		{
			armed=0;
		}
		
		if(armed == 0 && RC_CH[2] < 1500)
		{
			armed_flag=1;
		}	
				
	
	
	}

	
	
	
}

void TIM_RC_Init(void)
{
	GPIO_InitTypeDef 					GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_ICInitTypeDef 			  TIM5_ICInitStructure;	
	TIM_ICInitTypeDef 			  TIM4_ICInitStructure;
	NVIC_InitTypeDef 					NVIC_InitStructure;


	//设置NVIC中TIM4中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//通道设置为TIM4
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//响应3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//打开中断通道
	NVIC_Init(&NVIC_InitStructure);//写入配置
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;//通道设置为TIM4
	NVIC_Init(&NVIC_InitStructure);//写入配置
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM5, ENABLE);//使能TIM4时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,ENABLE);//GPIOB 时钟开启


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//GPIOB 6 7 8 9 口配置
	GPIO_ResetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);
	
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1);
	

	TIM_TimeBaseStructure.TIM_Period = 20000-1;
  TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1);//时钟预分频数 72M/72(1M/20000) = 50HZ
 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分频系数：不分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式(0->?)
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);//配置TIM4
	
	//初始化TIM4输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_4; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
//////////////////////////////
	TIM_TimeBaseStructure.TIM_Period = 20000-1;
  TIM_TimeBaseStructure.TIM_Prescaler = (72 - 1);//时钟预分频数 72M/72(1M/20000) = 50HZ
 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分频系数：不分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式(0->?)
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);//配置TIM
	
	//初始化TIM4输入捕获参数
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM5, &TIM5_ICInitStructure);
	
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM5, &TIM5_ICInitStructure);	
	
	
	
	TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);//允许CC1IE捕获中断	 
	TIM_ITConfig(TIM4,TIM_IT_CC2,ENABLE);//允许CC2IE捕获中断	
	TIM_ITConfig(TIM4,TIM_IT_CC3,ENABLE);//允许CC3IE捕获中断	
	TIM_ITConfig(TIM4,TIM_IT_CC4,ENABLE);//允许CC4IE捕获中断
	
	TIM_ITConfig(TIM5,TIM_IT_CC1,ENABLE);//允许CC1IE捕获中断	 
	TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//允许CC2IE捕获中断		
	
	
	TIM_Cmd(TIM4,ENABLE );//使能定时器4
	
	TIM_Cmd(TIM5,ENABLE );//使能定时器5
}

u8  TIM5CH1_CAPTURE_STA=0;	//通道1输入捕获状态		  用高两位做捕获标志，低六位做溢出计数  				
u16	TIM5CH1_CAPTURE_UPVAL;	//通道1输入捕获值
u16	TIM5CH1_CAPTURE_DOWNVAL;//通道1输入捕获值
       
u8  TIM5CH2_CAPTURE_STA=0;	//通道2输入捕获状态		    				
u16	TIM5CH2_CAPTURE_UPVAL;	//通道2输入捕获值
u16	TIM5CH2_CAPTURE_DOWNVAL;//通道2输入捕获值


void TIM5_IRQHandler(void)
{
	////////////////////////////////////
	OSIntEnter();		//进入中断
	
	if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)//捕获1发生捕获事件
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);//清除捕获1标志位
		
		if(TIM5CH1_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM5CH1_CAPTURE_UPVAL = TIM_GetCapture1(TIM5);//获取上升沿的数据
			
			TIM5CH1_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM5CH1_CAPTURE_DOWNVAL = TIM_GetCapture1(TIM5);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM5CH1_CAPTURE_DOWNVAL<TIM5CH1_CAPTURE_UPVAL)
			{
				RC_CH[4] = 20000 - TIM5CH1_CAPTURE_UPVAL + TIM5CH1_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[4] = TIM5CH1_CAPTURE_DOWNVAL- TIM5CH1_CAPTURE_UPVAL;
			}
			
			TIM5CH1_CAPTURE_STA = 0;
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //设置为上升沿捕获
		}
	}	
	
	
	///////////////////////////////////////////////////////
	if(TIM_GetITStatus(TIM5, TIM_IT_CC2) == SET)//捕获2发生捕获事件
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_CC2);//清除捕获2标志位
		
		if(TIM5CH2_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM5CH2_CAPTURE_UPVAL = TIM_GetCapture2(TIM5);//获取上升沿的数据
			
			TIM5CH2_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM5CH2_CAPTURE_DOWNVAL = TIM_GetCapture2(TIM5);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM5CH2_CAPTURE_DOWNVAL<TIM5CH2_CAPTURE_UPVAL)
			{
				RC_CH[5] = 20000 - TIM5CH2_CAPTURE_UPVAL + TIM5CH2_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[5] = TIM5CH2_CAPTURE_DOWNVAL- TIM5CH2_CAPTURE_UPVAL;
			}
			TIM5CH2_CAPTURE_STA = 0;
			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //设置为上升沿捕获
		}
	}
	
	   
	OSIntExit();        //触发任务切换软中断
}


//定时器4通道输入捕获配置
u8  TIM4CH1_CAPTURE_STA=0;	//通道1输入捕获状态		  用高两位做捕获标志，低六位做溢出计数  				
u16	TIM4CH1_CAPTURE_UPVAL;	//通道1输入捕获值
u16	TIM4CH1_CAPTURE_DOWNVAL;//通道1输入捕获值

u8  TIM4CH2_CAPTURE_STA=0;	//通道2输入捕获状态		    				
u16	TIM4CH2_CAPTURE_UPVAL;	//通道2输入捕获值
u16	TIM4CH2_CAPTURE_DOWNVAL;//通道2输入捕获值

u8  TIM4CH3_CAPTURE_STA=0;	//通道3输入捕获状态		    				
u16	TIM4CH3_CAPTURE_UPVAL;	//通道3输入捕获值
u16	TIM4CH3_CAPTURE_DOWNVAL;//通道3输入捕获值

u8  TIM4CH4_CAPTURE_STA=0;	//通道4输入捕获状态		    				
u16	TIM4CH4_CAPTURE_UPVAL;	//通道4输入捕获值
u16	TIM4CH4_CAPTURE_DOWNVAL;//通道4输入捕获值

void TIM4_IRQHandler(void)
{
/////////
	 	 OSIntEnter();		//进入中断
	
	
	if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET)//捕获1发生捕获事件
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);//清除捕获1标志位
		
		if(TIM4CH1_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM4CH1_CAPTURE_UPVAL = TIM_GetCapture1(TIM4);//获取上升沿的数据
			
			TIM4CH1_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM4CH1_CAPTURE_DOWNVAL = TIM_GetCapture1(TIM4);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM4CH1_CAPTURE_DOWNVAL<TIM4CH1_CAPTURE_UPVAL)
			{
				RC_CH[0] = 20000 - TIM4CH1_CAPTURE_UPVAL + TIM4CH1_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[0] = TIM4CH1_CAPTURE_DOWNVAL- TIM4CH1_CAPTURE_UPVAL;
			}
			
			TIM4CH1_CAPTURE_STA = 0;
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //设置为上升沿捕获
		}
	}

/////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC2) == SET)//捕获2发生捕获事件
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);//清除捕获2标志位
		
		if(TIM4CH2_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM4CH2_CAPTURE_UPVAL = TIM_GetCapture2(TIM4);//获取上升沿的数据
			
			TIM4CH2_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM4CH2_CAPTURE_DOWNVAL = TIM_GetCapture2(TIM4);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM4CH2_CAPTURE_DOWNVAL<TIM4CH2_CAPTURE_UPVAL)
			{
				RC_CH[1] = 20000 - TIM4CH2_CAPTURE_UPVAL + TIM4CH2_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[1] = TIM4CH2_CAPTURE_DOWNVAL- TIM4CH2_CAPTURE_UPVAL;
			}
			TIM4CH2_CAPTURE_STA = 0;
			TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //设置为上升沿捕获
		}
	}

	
///////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC3) == SET)//捕获3发生捕获事件
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);//清除捕获3标志位
		
		if(TIM4CH3_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM4CH3_CAPTURE_UPVAL = TIM_GetCapture3(TIM4);//获取上升沿的数据
			
			TIM4CH3_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM4CH3_CAPTURE_DOWNVAL = TIM_GetCapture3(TIM4);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM4CH3_CAPTURE_DOWNVAL<TIM4CH3_CAPTURE_UPVAL)
			{
				RC_CH[2] = 20000 - TIM4CH3_CAPTURE_UPVAL + TIM4CH3_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[2] = TIM4CH3_CAPTURE_DOWNVAL- TIM4CH3_CAPTURE_UPVAL;
			}
			
			TIM4CH3_CAPTURE_STA = 0;
			TIM_OC3PolarityConfig(TIM4,TIM_ICPolarity_Rising); //设置为上升沿捕获
			
		}
	}
	
///////////////
	if(TIM_GetITStatus(TIM4, TIM_IT_CC4) == SET)//捕获4发生捕获事件
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);//清除捕获4标志位
		
		if(TIM4CH4_CAPTURE_STA == 0)//捕获到上升沿
		{
			TIM4CH4_CAPTURE_UPVAL = TIM_GetCapture4(TIM4);//获取上升沿的数据
			
			TIM4CH4_CAPTURE_STA = 1;		//标记以捕获到了上升沿
			TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
		else                        //捕获到下降沿 (已经捕获到一个完整的高电平脉冲！)
		{
			TIM4CH4_CAPTURE_DOWNVAL = TIM_GetCapture4(TIM4);//获取下降沿的数据
			
			//判读是否超出发生了溢出,计算高电平脉冲时间us
			if(TIM4CH4_CAPTURE_DOWNVAL<TIM4CH4_CAPTURE_UPVAL)
			{
				RC_CH[3] = 20000 - TIM4CH4_CAPTURE_UPVAL + TIM4CH4_CAPTURE_DOWNVAL;
			}
			else
			{
				RC_CH[3] = TIM4CH4_CAPTURE_DOWNVAL- TIM4CH4_CAPTURE_UPVAL;
			}
			
			TIM4CH4_CAPTURE_STA = 0;
			TIM_OC4PolarityConfig(TIM4,TIM_ICPolarity_Rising); //设置为上升沿捕获
			
		}
	}
	

	  OSIntExit();        //触发任务切换软中断

}

