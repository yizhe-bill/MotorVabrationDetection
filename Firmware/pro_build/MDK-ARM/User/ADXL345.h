#ifndef __ADXL345_H_
#define __ADXL345_H_
/*包含头文件*/
#include "spi.h"
#include "gpio.h"//因为要用PA4，做软控制
#include "usart.h"
#include "ds18b20.h"

/*位定义*/
#define SPI1_CS_GPIO_Port  GPIOA
#define SPI1_CS_Pin        GPIO_PIN_4

/*变量声明*/
/******************ADXL345寄存器命令定义**********************/
#define DEVICE_ID           0X00        //获取器件ID,0XE5
#define THRESH_TAP          0X1D        //敲击阀值
#define OFSX                0X1E	//x轴调整偏移值
#define OFSY                0X1F    //y轴调整偏移值
#define OFSZ                0X20    //z轴调整偏移值
#define DUR                 0X21    //敲击持续时间
#define Latent              0X22    //敲击延迟时间
#define Window              0X23    //敲击窗口时间
#define THRESH_ACK          0X24    //敲击阈值
#define THRESH_INACT        0X25    //空闲阈值
#define TIME_INACT          0X26    //空闲时间
#define ACT_INACT_CTL       0X27    //空闲检测控制
#define THRESH_FF           0X28    //自由落体阈值
#define TIME_FF             0X29    //自由落体时间
#define TAP_AXES            0X2A    //敲击轴控制
#define ACT_TAP_STATUS      0X2B    //敲击状态
#define BW_RATE             0X2C    //输出速率和带宽
#define POWER_CTL           0X2D    //电源控制
 
#define INT_ENABLE          0X2E    //中断使能
#define INT_MAP             0X2F    //中断映射
#define INT_SOURCE          0X30    //中断源
#define DATA_FORMAT        0X31 //数据格式控制
#define DATA_X0            0X32 //X轴数据0
#define DATA_X1            0X33 //X轴数据1
#define DATA_Y0            0X34 //Y轴数据0
#define DATA_Y1            0X35 //Y轴数据1
#define DATA_Z0            0X36 //Z轴数据0
#define DATA_Z1            0X37 //Z轴数据1
#define FIFO_CTL            0X38    //FIFO控制
#define FIFO_STATUS         0X39    //FIFO状态
 
#define I_M_DEVID      ((uint8_t)0XE5) //器件ID=0XE5
/******************ADXL345指令集**********************/

/*函数声明*/
//初始化
void ADXL345_Init(void);
//初始化校准
void ADXL345_Calibrate(int width) ;
//读取X轴数据
short ADXL345_XTest(void);
//读取Y轴数据
short ADXL345_YTest(void);
//读取Z轴数据
short ADXL345_ZTest(void);
//打印加速度值
void printAccelerationG(void);
//
void printTable(void);
//
void ADXL345_TEMP_Calibrate(void);
//读取ID值
uint8_t Get_Adxl345_ID(void);
#endif
