#include "24l01.h"
#include "delay.h"
#include "usart.h"

const u8 TX_ADDRESS[TX_ADR_WIDTH]={0x12,0x34,0x56,0x78,0x9a}; //发送地址
const u8 RX_ADDRESS[RX_ADR_WIDTH]={0x12,0x34,0x56,0x78,0x9a}; //发送地址

u8 SPI1_ReadWriteByte(u8 TxData)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		retry++;
		if(retry>200)return 0;
	}	  					
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
}


void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI1,ENABLE); 

} 

//spi1初始化 
//spi1上nrf24L01的初始化
//PA567 SCK MISO MOSI 复用推挽输出
//PA4 PC4 CE CS 推挽输出
//PC5 INT 
/*
   GND       3.3V
PA4 CE   PC4  CS
PA5 SCK  PA7 MOSI
PA6 MISO PC5 INT
*/
void SPI1_NRF24L01_Init(void)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE );//PORTC时钟使能 SPI1时钟使能 	

	
	GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_5  | GPIO_Pin_6 | GPIO_Pin_7;  //PA567 复用推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PA57复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 ;   			//PA4 PC4 推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		

	//设置输入，，，，，，，，
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;   			//PC5 输入 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 ;   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	GPIO_SetBits(GPIOA, GPIO_Pin_4);   	
	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI主机
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//定义波特率预分频的值:波特率预分频值为16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
	
	SPI1_ReadWriteByte(0xff);//启动传输		 

	SPI1_NRF24L01_CE=0; 			//使能24L01
	SPI1_NRF24L01_CSN=1;			//SPI片选取消  
	 		 	 
}



u8 SPI1_NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	//加上这一句，如果spi速度过快， 无法完成校验。。  
//	SPI1_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,buf,5);//写入5个字节的地址.	
	SPI1_NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}	 	 

u8 SPI1_NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
   	SPI1_NRF24L01_CSN=0;                 //使能SPI传输
  	status =SPI1_ReadWriteByte(reg);//发送寄存器号 
  	SPI1_ReadWriteByte(value);      //写入寄存器的值
  	SPI1_NRF24L01_CSN=1;                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}

u8 SPI1_NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
 	SPI1_NRF24L01_CSN = 0;          //使能SPI传输		
  	SPI1_ReadWriteByte(reg);   //发送寄存器号
  	reg_val=SPI1_ReadWriteByte(0XFF);//读取寄存器内容
  	SPI1_NRF24L01_CSN = 1;          //禁止SPI传输		    
  	return(reg_val);           //返回状态值
}	

u8 SPI1_NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;	       
  	SPI1_NRF24L01_CSN = 0;           //使能SPI传输
  	status=SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI1_ReadWriteByte(0XFF);//读出数据
  	SPI1_NRF24L01_CSN=1;       //关闭SPI传输
  	return status;        //返回读到的状态值
}

u8 SPI1_NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	SPI1_NRF24L01_CSN = 0;          //使能SPI传输
  	status = SPI1_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI1_ReadWriteByte(*pBuf++); //写入数据	 
  	SPI1_NRF24L01_CSN = 1;       //关闭SPI传输
  	return status;          //返回读到的状态值
}				   

extern u8 NRF_payload_buf[16];
extern float OUT_P,OUT_I,OUT_D;	
extern float IN_P,IN_I,IN_D;   

u8 SPI1_NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
// 	SPI1_SetSpeed(SPI_BaudRatePrescaler_16);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	SPI1_NRF24L01_CE=0;
  	SPI1_NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	SPI1_NRF24L01_CE=1;//启动发送	   
	while(SPI1_NRF24L01_IRQ!=0);//等待发送完成
	sta=SPI1_NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	
	if(sta&MAX_TX)//达到最大重发次数
	{
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta == 46)//0010 1110 发送中断置位 RX FIFO空
	{
		return TX_OK;
	}
	if(sta == 32)//0010 0000 发送中断置位 RX FIFO 通道0 
	{
		u8 rx_len = SPI1_NRF24L01_Read_Reg(0x60);
		if(rx_len<33)
		{
			// read receive payload from RX_FIFO buffer
			//0110 0001 读取接收端发送的过载数据 过载数据从FIFO读出后被删除。 用在RX模式？？？
			SPI1_NRF24L01_Read_Buf(RD_RX_PLOAD,NRF_payload_buf,rx_len);// read receive payload from RX_FIFO buffer
			
			if(NRF_payload_buf[0] == 0xaa && NRF_payload_buf[1] == 0xaa)
			{
				OUT_P=NRF_payload_buf[2] 	/ 10.0;
				OUT_I=NRF_payload_buf[3]	/ 10.0;
				OUT_D=NRF_payload_buf[4]	/ 10.0;
				
				IN_P=NRF_payload_buf[5]	/ 10.0;
				IN_I=NRF_payload_buf[6]	/ 10.0;
				IN_D=NRF_payload_buf[7]	/ 10.0;
				
				
			}
			
		}
		else 
		{
			//1110 0010 删除接收端 FIFO数据  用于RX模式？
			SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);//清空缓冲区
		}
	}

	
	return 0xff;//其他原因发送失败
}




void SPI1_NRF24l01_Mode(u8 model)
{
	SPI1_NRF24L01_CE = 0;
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);	//写RX节点地址 
	SPI1_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH); 		//写TX节点地址  
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01); 													//EN_AA 			0000 0001 使能通道0的自动应答 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);											//EN_RXADDR 	0000 0001 使能通道0的接收地址 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x1a);											//SETUP_RETR  0001 1010 设置自动重发间隔时间:500us;最大自动重发次数:10次 
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);														//设置RF通道为CHANAL
	SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f); 												//设置TX发射参数,0db增益,2Mbps,低噪声增益开启
	//NRF_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x07); 												//设置TX发射参数,0db增益,1Mbps,低噪声增益开启
/////////////////////////////////////////////////////////
	if(model==RX_MODE)				//RX
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);								//选择通道0的有效数据宽度 
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 //CONFIG 0000 1111 IRQ收发完成中断开启,16位CRC,主接收
	}
	else if(model==TX_MODE)		//TX
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);								//选择通道0的有效数据宽度 
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ收发完成中断开启,16位CRC,主发送
	}
	else if(model==RX2_MODE)		//RX2
	{
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 // IRQ收发完成中断开启,16位CRC,主接收
		
		SPI1_ReadWriteByte(0x50);
		SPI1_ReadWriteByte(0x73);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	}
	else								//TX2
	{
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ收发完成中断开启,16位CRC,主发送
		SPI1_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI1_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		
		SPI1_ReadWriteByte(0x50);
		SPI1_ReadWriteByte(0x73);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI1_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	}
	SPI1_NRF24L01_CE = 1;
}












