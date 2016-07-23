#include "sys.h"
#include "control.h"
#include "sysconfig.h"

#define Moto_PwmMax 1260  //1260
#define Moto_PwmMin 540		//540

void limit(float x,float y)
{
	if(x>y)  x=y;		
	if(x<-y) x=-y;		
}
u16 speed0=0,speed1=0,speed2=0,speed3=0;

float OUT_P=0.0,OUT_I=0,OUT_D=0;	
float IN_P=0,IN_I=0,IN_D=0.0;   //限制自由度的情况下， 内环P取0.7

extern S_FLOAT_PRY 		Q_ANGLE,Pre_Q_ANGLE;			//四元数计算出的角度						
extern S_INT16_RC 		RC_GET;
extern int16_t armed;			
extern S_FLOAT_XYZ   Q_GYRO;
void PIDPID( )//角速度传入单位角度每秒
{
	

	float 				OUTPID_error_roll,       	  OUTPID_error_pitch;		
	static	float OUTPID_integration_roll,		OUTPID_integration_pitch;	
	float 				OUTPID_out_roll,            OUTPID_out_pitch;		
                                          			
	float 				INPID_error_roll,								INPID_error_pitch;
	static 	float INPID_integration_roll,					INPID_integration_pitch;
	static	float INPID_p1_roll,INPID_p2_roll,		INPID_p1_pitch,INPID_p2_pitch;
	
	float 				output_roll,		output_pitch;

	
	
/////外环：角度误差PI后作为期望角速度输入内环/////////////
	
	/////////////roll方向//////////////////////////////////
	OUTPID_error_roll = RC_GET.ROLL - Q_ANGLE.rol ;
	limit(OUTPID_error_roll,45);

	OUTPID_integration_roll += OUTPID_error_roll;
	limit(OUTPID_integration_roll,200);
	
	OUTPID_out_roll =	  OUT_P * OUTPID_error_roll
										+ OUT_I * OUTPID_integration_roll;
	
	//////////////pitch方向//////////////////////////
	OUTPID_error_pitch = RC_GET.PITCH - Q_ANGLE.pit;
	limit(OUTPID_error_pitch,45);

	OUTPID_integration_pitch += OUTPID_error_pitch;
	limit(OUTPID_integration_pitch,200);
	
	OUTPID_out_pitch =	  OUT_P * OUTPID_error_pitch
											+ OUT_I * OUTPID_integration_pitch;	
//////////内环：角速度误差PID后输出PWM//////////////////				


	/////////////roll方向//////////////////////////////////
	INPID_error_roll = OUTPID_out_roll - Q_GYRO.Y;
	INPID_p1_roll =	INPID_p2_roll;
	INPID_p2_roll = INPID_error_roll;	
	INPID_integration_roll += INPID_error_roll;			
	limit(INPID_integration_roll,2000);

	output_roll = 	IN_P * INPID_error_roll
								+ IN_I * INPID_integration_roll
								+ IN_D * (INPID_p2_roll - INPID_p1_roll);
	limit(output_roll,250);	
	
	
	//////////////pitch方向/////////////////////////
	INPID_error_pitch = OUTPID_out_pitch - Q_GYRO.X;
	INPID_p1_pitch =	INPID_p2_pitch;
	INPID_p2_pitch = INPID_error_pitch;
	INPID_integration_pitch += INPID_error_pitch;			
	limit(INPID_integration_pitch,2000);
	
	
	output_pitch = 	IN_P * INPID_error_pitch
								+ IN_I * INPID_integration_pitch
								+ IN_D * (INPID_p2_pitch - INPID_p1_pitch);			
	limit(output_pitch,250);	
///////////////////////////////////////////////////////////


	
//	speed0 = 600 + rc_get->THROTTLE - out_r + out_p;
//	speed1 = 600 + rc_get->THROTTLE - out_r - out_p;	
//	speed2 = 600 + rc_get->THROTTLE + out_r - out_p;
//	speed3 = 600 + rc_get->THROTTLE + out_r + out_p;
		
	// PC6789 	

//	speed0 = 600 + RC_GET.THROTTLE + output_roll + output_pitch;
//	speed1 = 600 + RC_GET.THROTTLE - output_roll - output_pitch;	
//	speed2 = 600 + RC_GET.THROTTLE - output_roll - output_pitch;
//	speed3 = 600 + RC_GET.THROTTLE + output_roll + output_pitch;

	speed0 = 600 + RC_GET.THROTTLE + output_roll;
	speed1 = 600 + RC_GET.THROTTLE - output_roll;	
	speed2 = 600 + RC_GET.THROTTLE - output_roll;
	speed3 = 600 + RC_GET.THROTTLE + output_roll;

	
	if((Q_ANGLE.rol)>30) 	armed=0;
	if((Q_ANGLE.rol)<-30) 	armed=0;	
	if((Q_ANGLE.pit)>30) 	armed=0;
	if((Q_ANGLE.pit)<-30) 	armed=0;
	

	if(armed==0)
	{			
		speed0=0;
		speed1=0;
		speed2=0;
		speed3=0;
		OUTPID_integration_roll=0;
		OUTPID_integration_pitch=0;
		INPID_integration_roll=0;
		INPID_integration_pitch=0;
	}
	if(speed0>Moto_PwmMax)	speed0 = Moto_PwmMax;
	if(speed1>Moto_PwmMax)	speed1 = Moto_PwmMax;
	if(speed2>Moto_PwmMax)	speed2 = Moto_PwmMax;
	if(speed3>Moto_PwmMax)	speed3 = Moto_PwmMax;
	
	if(speed0<Moto_PwmMin)	speed0 = Moto_PwmMin;
	if(speed1<Moto_PwmMin)	speed1 = Moto_PwmMin;
	if(speed2<Moto_PwmMin)	speed2 = Moto_PwmMin;
	if(speed3<Moto_PwmMin)	speed3 = Moto_PwmMin;
	
	                                                                                                                                                                                                       
  //0.9ms对应0油门，2.1ms对应100油门，总油门为1200， 670~1260		
	TIM_SetCompare1(TIM3,speed0);   //PC6
	TIM_SetCompare2(TIM3,speed1);		//PC7
	TIM_SetCompare3(TIM3,speed2);		//PC8
	TIM_SetCompare4(TIM3,speed3);		//PC9
}



//void PIDPID(S_FLOAT_PRY* angle,S_INT16_XYZ* gyro,S_INT16_RC * rc_get )//角速度传入单位角度每秒
//{
//	float 				OUT_e_p,OUT_e_r;
//	static	float OUT_i_p,OUT_i_r;	
//	float 				OUT_out_p,OUT_out_r;

//	float 				IN_e_p,IN_e_r;
//	static 	float IN_i_p,IN_i_r;	
//	static	float IN_p1_p,IN_p2_p,IN_p1_r,IN_p2_r;
//	
//	float 				out_p,out_r;

///////外环：角度误差PI后作为期望角速度输入内环/////////////
//	OUT_e_r = rc_get->ROLL - angle->rol ;
//	limit(OUT_e_r,45);

//	OUT_i_r += OUT_e_r;
//	limit(OUT_e_r,200);
//	
//	OUT_out_r =	  OUT_P * OUT_e_r
//							+ OUT_I * OUT_i_r;
//	
//	
//	OUT_e_p = rc_get->PITCH - angle->pit;
//	limit(OUT_e_p,45);

//	OUT_i_p += OUT_e_p;
//	limit(OUT_i_p,200);
//	
//	OUT_out_p =	  OUT_P * OUT_e_p
//							+ OUT_I * OUT_i_p;	
////////////内环：角速度误差PID后输出PWM//////////////////				
//	IN_e_r = OUT_out_r - gyro->X ;
//	IN_p1_r =	IN_p2_r;
//	IN_p2_r = IN_e_r;	
//	IN_i_r += IN_e_r;			
//	limit(IN_i_r,2000);

//	out_r = 	IN_P * IN_e_r
//					+ IN_I * IN_i_r
//					+ IN_D * (IN_p2_r - IN_p1_r);
//	limit(out_r,250);	


//	IN_e_p = OUT_out_p - gyro->Y ;
//	IN_p1_p =	IN_p2_p;
//	IN_p2_p = IN_e_p;
//	IN_i_p += IN_e_p;			
//	limit(IN_i_p,2000);
//	
//	
//	out_p = 	IN_P * IN_e_p
//					+ IN_I * IN_i_p
//					+ IN_D * (IN_p2_p - IN_p1_p);			
//	limit(out_p,250);	

//	
////	speed0 = 600 + rc_get->THROTTLE - out_r + out_p;
////	speed1 = 600 + rc_get->THROTTLE - out_r - out_p;	
////	speed2 = 600 + rc_get->THROTTLE + out_r - out_p;
////	speed3 = 600 + rc_get->THROTTLE + out_r + out_p;
//		
//	speed0 = 600 + rc_get->THROTTLE - out_r ;
//	speed1 = 600 + rc_get->THROTTLE - out_r ;	
//	speed2 = 600 + rc_get->THROTTLE + out_r ;
//	speed3 = 600 + rc_get->THROTTLE + out_r ;

////	speed0 = 600  - out_r + out_p;
////	speed1 = 600  - out_r - out_p;	
////	speed2 = 600  + out_r - out_p;
////	speed3 = 600  + out_r + out_p;
//	
//	if((Q_ANGLE.rol)>30) 	armed=0;
//	if((Q_ANGLE.rol)<-30) 	armed=0;	
//	if((Q_ANGLE.pit)>30) 	armed=0;
//	if((Q_ANGLE.pit)<-30) 	armed=0;
//	
////	if((gyro->X)<-200) 	armed=0;	
////	if((gyro->X)>200) 	armed=0;		
////	if((gyro->Y)<-200) 	armed=0;		
////	if((gyro->Y)>200) 	armed=0;	
//	
////	if(speed0>1500) 	armed=0;	
////	if(speed1>1500) 	armed=0;		
////	if(speed2>1500) 	armed=0;	
////	if(speed3>1500) 	armed=0;				
//	
////	if((Q_ANGLE.rol)>5) 	armed=0;
////	if((Q_ANGLE.rol)<-5) 	armed=0;	
////	if((Q_ANGLE.pit)>5) 	armed=0;
////	if((Q_ANGLE.pit)<-5) 	armed=0;


//	if(armed==0)
//	{			
//		speed0=0;
//		speed1=0;
//		speed2=0;
//		speed3=0;
//		OUT_i_p=0;
//		OUT_i_r=0;
//		IN_i_p=0;
//		IN_i_r=0;
//	}
//	if(speed0>Moto_PwmMax)	speed0 = Moto_PwmMax;
//	if(speed1>Moto_PwmMax)	speed1 = Moto_PwmMax;
//	if(speed2>Moto_PwmMax)	speed2 = Moto_PwmMax;
//	if(speed3>Moto_PwmMax)	speed3 = Moto_PwmMax;
//	
//	if(speed0<Moto_PwmMin)	speed0 = Moto_PwmMin;
//	if(speed1<Moto_PwmMin)	speed1 = Moto_PwmMin;
//	if(speed2<Moto_PwmMin)	speed2 = Moto_PwmMin;
//	if(speed3<Moto_PwmMin)	speed3 = Moto_PwmMin;
//	
//	
//  //0.9ms对应0油门，2.1ms对应100油门，总油门为1200， 670~1260		
//	TIM_SetCompare1(TIM3,speed0);   //PC6
//	TIM_SetCompare2(TIM3,speed1);		//PC7
//	TIM_SetCompare3(TIM3,speed2);		//PC8
//	TIM_SetCompare4(TIM3,speed3);		//PC9
//}



