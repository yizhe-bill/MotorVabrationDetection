#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "gpio.h"

typedef enum {
    NO_KEY      = 0,   // 无按键按下
    KEY_UP      = 1,   // 上键按下
    KEY_DOWN    = 2,   // 下键按下
    KEY_ENTER   = 3,   // 确认键按下
    KEY_BACK    = 4    // 返回键按下
} Key_State_t;


void Key_Init(void);
uint8_t Key_GetNum(void);
void Key_Tick(void);

#endif
