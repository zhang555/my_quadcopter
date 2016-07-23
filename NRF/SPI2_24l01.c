#include "24l01.h"
#include "delay.h"

#include "usart.h"


u8 SPI2_ReadWriteByte(u8 TxData)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		retry++;
		if(retry>200)return 0;
	}	  					
	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据					    
}


void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); 

} 
//SPI2 初始化
//SPI2 nrf24l01 初始化
//PB13 14 15 SCK MISO MOSI 复用推挽输出
//PG67 CE CS 推挽输出
//PG8 IRQ
void SPI2_NRF24L01_Init(void)
{ 	
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);	 //使能PB,D,G端口时钟
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 	    	
	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //PB12上拉 防止W25X的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化指定IO
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);//上拉				
 	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 //PD2推挽输出上拉   禁止SD卡的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_SetBits(GPIOD,GPIO_Pin_2);//初始化指定IO
	*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//PG6 7 推挽 	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
 	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化指定IO
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
 	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化指定IO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

//GPIO_ResetBits(GPIOG,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);//PG6,7,8上拉					

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI主机
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;		//定义波特率预分频的值:波特率预分频值为16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	
	SPI2_ReadWriteByte(0xff);//启动传输		 
 
			 
	SPI2_NRF24L01_CE=0; 			//使能24L01
	SPI2_NRF24L01_CSN=1;			//SPI片选取消  
	 		 	 
}



u8 SPI2_NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   	 
	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,buf,5);//写入5个字节的地址.	
	SPI2_NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}	 	 

u8 SPI2_NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
   	SPI2_NRF24L01_CSN=0;                 //使能SPI传输
  	status =SPI2_ReadWriteByte(reg);//发送寄存器号 
  	SPI2_ReadWriteByte(value);      //写入寄存器的值
  	SPI2_NRF24L01_CSN=1;                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}

u8 SPI2_NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val;	    
 	SPI2_NRF24L01_CSN = 0;          //使能SPI传输		
  	SPI2_ReadWriteByte(reg);   //发送寄存器号
  	reg_val=SPI2_ReadWriteByte(0XFF);//读取寄存器内容
  	SPI2_NRF24L01_CSN = 1;          //禁止SPI传输		    
  	return(reg_val);           //返回状态值
}	

u8 SPI2_NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;	       
  	SPI2_NRF24L01_CSN = 0;           //使能SPI传输
  	status=SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);//读出数据
  	SPI2_NRF24L01_CSN=1;       //关闭SPI传输
  	return status;        //返回读到的状态值
}

u8 SPI2_NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	SPI2_NRF24L01_CSN = 0;          //使能SPI传输
  	status = SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI2_ReadWriteByte(*pBuf++); //写入数据	 
  	SPI2_NRF24L01_CSN = 1;       //关闭SPI传输
  	return status;          //返回读到的状态值
}				   

u8 SPI2_NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;
// 	SPI2_SetSpeed(SPI_BaudRatePrescaler_8);//spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	SPI2_NRF24L01_CE=0;
  	SPI2_NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	SPI2_NRF24L01_CE=1;//启动发送	   
	while(SPI2_NRF24L01_IRQ!=0);//等待发送完成
	sta=SPI2_NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}

//伪双工接收方加载一个ack数据包
void SPI2_NRF24L01_TxPacket_AP(uint8_t * tx_buf)
{	
	SPI2_NRF24L01_CE=0;	 //StandBy I模式	
	//command word 1010 1xxx 最大通道4 通道0装载数据
	SPI2_NRF24L01_Write_Buf(0xa8, tx_buf, TX_PLOAD_WIDTH); 			
	SPI2_NRF24L01_CE=1;	 //置高CE
}
u8 SPI2_NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
//	SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=SPI2_NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		SPI2_NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		SPI2_NRF24L01_TxPacket_AP(rxbuf);
//		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}					
//接收加载的数据包。
void SPI2_NRF24L01_RxPacket_AP(u8 *rxbuf)
{
	u8 sta;
	sta = SPI2_NRF24L01_Read_Reg(READ_REG_NRF + STATUS);
	if(sta & RX_OK)
	{
		u8 rx_len = SPI2_NRF24L01_Read_Reg(0x60);
		if(rx_len<33)
		{
			SPI2_NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,rx_len);// read receive payload from RX_FIFO buffer
			usart1_send_char(0x55);	
			
		}
		else 
		{
			SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);//清空缓冲区
		}
	}
	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + STATUS, sta);
}

void SPI2_NRF24L01_RX_Mode(void)
{
	SPI2_NRF24L01_CE=0;	  
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);    //使能通道0的自动应答    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);//使能通道0的接收地址  	 
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);	     //设置RF通信频率		  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  SPI2_NRF24L01_CE = 1; //CE为高,进入接收模式 
}			


void SPI2_NRF24L01_TX_Mode(void)
{														 
	SPI2_NRF24L01_CE=0;	    
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //使能通道0的自动应答    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //使能通道0的接收地址  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0xfa);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //设置RF通道为40
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	SPI2_NRF24L01_CE=1;//CE为高,10us后启动发送
}		  



void SPI2_TX2_Mode(void)
{
	SPI2_NRF24L01_CE=0;	    
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
  	SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01);     //使能通道0的自动应答    
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01); //使能通道0的接收地址  
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0xfa);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);       //设置RF通道为40
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	
		//伪双工发送设置
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0e);   		 // IRQ收发完成中断开启,16位CRC,主发送
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI2_ReadWriteByte(0x50);
		SPI2_ReadWriteByte(0x73);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	SPI2_NRF24L01_CE=1;//CE为高,10us后启动发送

}
void SPI2_RX2_Mode(void)
{
	SPI2_NRF24L01_CE=0;
		SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);	//写RX节点地址 
		SPI2_NRF24L01_Write_Buf(WRITE_REG_NRF+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH); 		//写TX节点地址  
		   
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_AA,0x01); 													//使能通道0的自动应答 
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+EN_RXADDR,0x01);											//使能通道0的接收地址 
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_CH,40);														//设置RF通道为CHANAL	
		   
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+RF_SETUP,0x0f); 												//设置TX发射参数,0db增益,2Mbps,低噪声增益开启	
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+SETUP_RETR,0x1a);											//设置自动重发间隔时间:500us;最大自动重发次数:10次 
		//伪双工配置
		SPI2_NRF24L01_Write_Reg(FLUSH_TX,0xff);
		SPI2_NRF24L01_Write_Reg(FLUSH_RX,0xff);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF + CONFIG, 0x0f);   		 // IRQ收发完成中断开启,16位CRC,主接收
		SPI2_ReadWriteByte(0x50);
		SPI2_ReadWriteByte(0x73);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1c,0x01);
		SPI2_NRF24L01_Write_Reg(WRITE_REG_NRF+0x1d,0x06);
	SPI2_NRF24L01_CE = 1;
}	







