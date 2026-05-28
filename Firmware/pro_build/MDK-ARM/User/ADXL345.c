#include "ADXL345.h"
#include "stdio.h"

#define SET_SPI_CS_H	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);//SPI CS 软触发  高
#define SET_SPI_CS_L	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);//SPI CS 软触发 低

/********************************************************************
   * @brief  : 写字节
   * @param  : 地址，数据
   * @retval :
*********************************************************************/
void ADXL345_Write(uint8_t addr, uint8_t value)
{
	addr &= 0x3F;
	SET_SPI_CS_L;

//使用DMA传输SPI数据
	HAL_SPI_Transmit_DMA(&hspi1, &addr, 1);  // HAL_SPI_Transmit(&hspi1, &addr, 1, 10);
	HAL_SPI_Transmit_DMA(&hspi1, &value, 1); // HAL_SPI_Transmit(&hspi1, &value, 1, 10);
//等待传输完成
	//while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	
	SET_SPI_CS_H;
}
/********************************************************************
   * @brief  : 读字节
   * @param  : 地址，数据
   * @retval :
*********************************************************************/
void ADXL345_Read(uint8_t addr, uint8_t *value)
{
	addr &= 0x3F;	
	addr |= (0x80);
	SET_SPI_CS_L;

//使用DMA传输SPI数据
	HAL_SPI_Transmit_DMA(&hspi1, &addr, 1);  // HAL_SPI_Transmit(&hspi1, &addr, 1, 10);
	HAL_SPI_Receive_DMA(&hspi1, value, 1);   // HAL_SPI_Receive(&hspi1, value, 1, 10);

	SET_SPI_CS_H;
}
/********************************************************************
   * @brief  : 读取ID值
   * @param  :
   * @retval :返回读取到的ID值
*********************************************************************/
uint8_t Get_Adxl345_ID(void)
{
	uint8_t DEVICEID = 0x00;
	uint8_t result = 0;
	ADXL345_Read(DEVICEID, &result);

	return result;
}
/********************************************************************
   * @brief  : 初始化传感器
   * @param  :
   * @retval :打印正确值
*********************************************************************/
void ADXL345_Init(void)
{
	while (Get_Adxl345_ID() != I_M_DEVID)//e5
	{
		printf_DMA("ADXL345 Init Fail:%x\r\n", Get_Adxl345_ID());
		HAL_Delay(1000);
	}

	ADXL345_Write(INT_ENABLE, 0x00);    //中断失能
	ADXL345_Write(DATA_FORMAT, 0x0B);   //右对齐？
	ADXL345_Write(BW_RATE, 0x10|0X0F);       //输出速率，0~15_0X0~0XF,最大3200HZ
	ADXL345_Write(POWER_CTL, 0x08);     //自动睡眠
	ADXL345_Write(INT_ENABLE, 0x14);

	printf_DMA("ADXL345 Init Success\r\nThe ID is %x\r\n",Get_Adxl345_ID());
}
/*******************************************************************************
* 函数名     ：ADXL345校准函数
* 参数功能 ：width --> 平滑窗口宽度
* 返回值	   ：
*******************************************************************************/

void ADXL345_Calibrate(int width) {
	uint8_t x0, x1, y0, y1, z0, z1;
	int16_t sumx = 0, sumy = 0, sumz = 0;	
    int16_t x_avg;
    int16_t y_avg;
    int16_t z_avg;
    
    // 1. 清除偏移设置
    ADXL345_Write(OFSX, 0);
    ADXL345_Write(OFSY, 0);
    ADXL345_Write(OFSZ, 0);
	// 2. 读取原始测量值_采用平滑窗口处理
	//简单移动平滑滤波
    for(int i=0;i<width;i++)
	{
		ADXL345_Read(DATA_X0, &x0);
		ADXL345_Read(DATA_X1, &x1);
		ADXL345_Read(DATA_Y0, &y0);
		ADXL345_Read(DATA_Y1, &y1);
		ADXL345_Read(DATA_Z0, &z0);
		ADXL345_Read(DATA_Z1, &z1);
		//拼接数据————详情见datasheet——P27
		int16_t x_data = (x1 << 8) | x0;
		int16_t y_data = (y1 << 8) | y0;
		int16_t z_data = (z1 << 8) | z0;
		//求和
		sumx += x_data;
		sumy += y_data;
		sumz += z_data;
	}
	//求平均数
	x_avg = sumx / width;
	y_avg = sumy / width;
	z_avg = sumz / width;
	//printf("Average: x_avg=%d, y_avg=%d, z_avg=%d\r\n", x_avg, y_avg, z_avg);
    
	// 3. 计算偏移值(理想值：0, 0, -256)，
	//全分辨率，16g下，标度因子：256LSB/g，即0.00390625g/LSB，3.90625mg/LSB
	// 0g对应的值为512，-1g对应的值为-256，公式：偏置值 = -（实测平均值 - 理想值） / 标度因子
	int8_t ofs_x = -(x_avg - 0) / 3.90625;
	int8_t ofs_y = -(y_avg - 0) / 3.90625;
	int8_t ofs_z = -(z_avg - 512) / 3.90625;
    
	//4. 写入偏移寄存器 确保在-128~+127范围内
    ADXL345_Write(OFSX, ofs_x & 0XFF);
    ADXL345_Write(OFSY, ofs_y & 0XFF);
    ADXL345_Write(OFSZ, ofs_z & 0XFF);
    // 验证输出
    //printf("Calibrated: OFSX=%d, OFSY=%d, OFSZ=%d\r\n", ofs_x, ofs_y, ofs_z);
}
/*******************************************************************************
* 函数名     ：打印三轴加速度（g单位）
* 参数功能 ：
* 返回值	   ：
*******************************************************************************/
void printAccelerationG()
{
    //优化后（减少变量赋值，以减少在VOFA中的毛刺）
        printf("%.5f,%.5f,%.5f\r\n", (float)ADXL345_XTest()*0.00390625f, (float)ADXL345_YTest()*0.00390625f, (float)ADXL345_ZTest()*0.00390625f);
}

//读取X轴数据
short ADXL345_XTest(void)
{
	short x;
    
	uint8_t xl, xh;

	ADXL345_Read(DATA_X0, &xl);
	ADXL345_Read(DATA_X1, &xh);

	x = (short)(((uint16_t)xh << 8) + xl);

	//printf("X:%d\r\n", x);
    return  x;
}
//读取Y轴数据
short ADXL345_YTest(void)
{
	short Y;
	uint8_t Yl, Yh;

	ADXL345_Read(DATA_Y0, &Yl);
	ADXL345_Read(DATA_Y1, &Yh);

	Y = (short)(((uint16_t)Yh << 8) + Yl);

	//printf("Y:%d\r\n", Y);
    return Y;
}
//读取Z轴数据
short ADXL345_ZTest(void)
{
	short Z;
	uint8_t Zl, Zh;

	ADXL345_Read(DATA_Z0, &Zl);
	ADXL345_Read(DATA_Z1, &Zh);

	Z = (short)(((uint16_t)Zh << 8) + Zl);

	//printf("Z:%d\r\n", Z);
    return Z;
}
/*******************************************************************************
* 函数名     ：打印温度-三轴数据表格
* 参数功能 ：表格格式——温度，X，Y，Z
* 返回值	   ：此函数用于数据离线分析阶段
*******************************************************************************/
void printTable()
{
	printf("Temperature\tX\tY\tZ\r\n");
	printf("------------------------------------------------\r\n");
	for (unsigned int i = 0; i < 1000; i++)
	{
		printf("%.2f\t%.2f\t%.2f\t%.2f\r\n", (float)DS18B20_Get_Temperature(), (float)ADXL345_XTest(), (float)ADXL345_YTest(), (float)ADXL345_ZTest());
		//HAL_Delay(1);//共计65536*1ms
	}
}

_Bool flag_first=1;
short TempChange,PreTemp;

void ADXL345_TEMP_Calibrate()
{
  if(flag_first){ //第一次
    PreTemp=DS18B20_Get_Temperature();  //先记录一个历史值
    //TempChange=DS18B20_Get_Temperature()-PreTemp;//不计算
    flag_first=0;}//以后再也不用此标志位
  else {  //第一次之后
    if((DS18B20_Get_Temperature()-PreTemp)>5){  //通过现在的温度和历史值做差
      ADXL345_Calibrate(600); //如果相差过大就进行校准
      PreTemp=DS18B20_Get_Temperature();  //同时刷新历史值
    }//如果没有较大差别我们就不做任何计算，也不刷新历史值
  }
}
