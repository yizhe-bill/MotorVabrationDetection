#ifndef __MY_DELAY_H_
#define __MY_DELAY_H_
/*包含头文件*/
#include "main.h"                  // Device header

/*位定义*/
#define SYSTEM_SUPPORT_OS		0		//定义系统文件夹是否支持OS

/*变量声明*/

/*函数声明*/
void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);

#endif
