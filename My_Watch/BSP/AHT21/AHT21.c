#include "AHT21.h"
/** 
 * @brief  AHT21 输入电压范围介于2.2V与5.5V之间
 * @note   时序操作
 * @note   1. 上电后要等待不少于100ms
*/
#define AHT_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE()
/** 
 * @brief  AHT21初始化GPIO口
 * @note   None
*/
iic_bus_t AHT_bus = 
{
	.IIC_SDA_PORT = GPIOB,
	.IIC_SCL_PORT = GPIOB,
	.IIC_SDA_PIN  = GPIO_PIN_13,
	.IIC_SCL_PIN  = GPIO_PIN_14,
};
/** 
 * @brief  AHT21读取状态位
 * @param  None
 * @retval 读取到的状态位
 * @Note   发送0x71读取状态位 
*/
uint8_t AHT_Read_Status(void)
{
	uint8_t Byte_first;	
	IICStart(&AHT_bus);							//起始信号
	IICSendByte(&AHT_bus,0x71);					//发送读状态位命令0x71
	IICWaitAck(&AHT_bus);						//等待应答
	Byte_first = IICReceiveByte(&AHT_bus);      //接收状态位
	IICSendNotAck(&AHT_bus);					//发送非应答信号
	IICStop(&AHT_bus);							//停止信号
	return Byte_first;							//返回状态位
}
/** 
 * @brief  校验AHT21是否处于正常工作状态
 * @param  None
 * @retval 1:正常工作状态 0:异常工作状态
 * @Note   val = AHT_Read_Status()&0x80 == 0x08
*/
uint8_t AHT_Read_Cal_Enable(void)  //check cal enable bit 
{
	uint8_t val = 0;//ret = 0,
 
  val = AHT_Read_Status();
  if((val & 0x68)==0x08)  //check NOR mode 
		return 1;
  else  
		return 0;
}
/** 
 * @brief  AHT21复位
 * @param  None
 * @retval None
 * @Note   发送0xBA复位命令
*/
void AHT_Reset(void)//AHT21 send 0xBA reset call
{
	IICStart(&AHT_bus);
	IICSendByte(&AHT_bus,0x70);
	IICWaitAck(&AHT_bus);
	IICSendByte(&AHT_bus,0xBA);
	IICWaitAck(&AHT_bus);
	IICStop(&AHT_bus);
}

uint8_t AHT_Init(void)
{
	AHT_CLK_ENABLE;
	IICInit(&AHT_bus);
	
	delay_ms(40);
	
	if(AHT_Read_Status()&&0x08!=0x08)
	{
		IICStart(&AHT_bus);
		IICSendByte(&AHT_bus,0x70);
		IICWaitAck(&AHT_bus);
		IICSendByte(&AHT_bus,0xBE);
		IICWaitAck(&AHT_bus);
		IICSendByte(&AHT_bus,0x08);
		IICWaitAck(&AHT_bus);
		IICSendByte(&AHT_bus,0x00);
		IICWaitAck(&AHT_bus);
		IICStop(&AHT_bus);
		delay_ms(10);
	}
	//AHT_Reset();
	
	return 0;
}	
 
uint8_t AHT_Read(float *humi, float *temp)
{
	uint8_t cnt=5;
	uint8_t  Byte_1th=0;
	uint8_t  Byte_2th=0;
	uint8_t  Byte_3th=0;
	uint8_t  Byte_4th=0;
	uint8_t  Byte_5th=0;
	uint8_t  Byte_6th=0;
	uint32_t RetuData = 0;
	
	IICStart(&AHT_bus);
	IICSendByte(&AHT_bus,0x70);
	IICWaitAck(&AHT_bus);
	IICSendByte(&AHT_bus,0xAC);
	IICWaitAck(&AHT_bus);
	IICSendByte(&AHT_bus,0x33);
	IICWaitAck(&AHT_bus);
	IICSendByte(&AHT_bus,0x00);
	IICWaitAck(&AHT_bus);
	IICStop(&AHT_bus);	
	
	delay_ms(80);
	while(AHT_Read_Status()&0x80==0x80 && cnt)
	{
		delay_ms(5);
		cnt--;
		AHT_Read_Status();
	}
	if(!cnt)
	{return 1;}
	
	IICStart(&AHT_bus);
	IICSendByte(&AHT_bus,0x71);
	IICWaitAck(&AHT_bus);
	Byte_1th = IICReceiveByte(&AHT_bus);
	IICSendAck(&AHT_bus);
	Byte_2th = IICReceiveByte(&AHT_bus);
	IICSendAck(&AHT_bus);
	Byte_3th = IICReceiveByte(&AHT_bus);
	IICSendAck(&AHT_bus);
	Byte_4th = IICReceiveByte(&AHT_bus);
	IICSendAck(&AHT_bus);
	Byte_5th = IICReceiveByte(&AHT_bus);
	IICSendAck(&AHT_bus);
	Byte_6th = IICReceiveByte(&AHT_bus);
	IICSendNotAck(&AHT_bus);
	IICStop(&AHT_bus);
	//读取AHTSrh数据 共20位
	RetuData = (RetuData|Byte_2th)<<8;
	RetuData = (RetuData|Byte_3th)<<8;
	RetuData = (RetuData|Byte_4th);
	RetuData =RetuData >>4;
	//湿度计算公式
	*humi = (RetuData * 1000 >> 20);
	*humi /= 10;
	//读取温度数据 共20位
	RetuData = 0;
	//只去BYTE4后四位
	RetuData = (RetuData|(Byte_4th&0x0f))<<8;
	RetuData = (RetuData|Byte_5th)<<8;
	RetuData = (RetuData|Byte_6th);
	RetuData = RetuData&0xfffff;
	//温度计算公式
	*temp = ((RetuData * 2000 >> 20)- 500);
	*temp /= 10;
	
	return 0;
}
