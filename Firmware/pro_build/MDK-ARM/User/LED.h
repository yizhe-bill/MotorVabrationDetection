#ifndef __LED_H_
#define __LED_H_
/*包含头文件*/
#include "main.h"
#include "gpio.h"

/*宏定义*/
//在main.H中已有
/*变量声明*/
//用结构体列举灯的状态
typedef enum 
{
    LED_OFF = 0,
    LED_ON = 1,
    LED_BLINK = 2
}LED_State;
typedef struct
{
    GPIO_TypeDef *port; //灯的端口
    uint16_t pin;       //灯的引脚
    LED_State state; //灯的状态
} LED_TypeDef;
/*函数声明*/
void LED_Init(LED_TypeDef *led, GPIO_TypeDef *port, uint16_t pin);
void LED_SetState(LED_TypeDef *led, LED_State state);


#endif

