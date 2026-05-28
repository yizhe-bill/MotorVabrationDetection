#include "Key.h"                  // Device header


uint8_t Key_Num;

void Key_Init(void)
{
//已经通过CUBEMX配置，在GPIO.H；默认上拉
}

uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if (Key_Num)
	{
		Temp = Key_Num;
		Key_Num = 0;
		return Temp;
	}
	return 0;
}

uint8_t Key_GetState(void)
{//下拉出发
	if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == 0)		    return KEY_UP;
	if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == 0)		return KEY_DOWN;
	if (HAL_GPIO_ReadPin(KEY_ENTER_GPIO_Port, KEY_ENTER_Pin) == 0)	return KEY_ENTER;
	if (HAL_GPIO_ReadPin(KEY_BACK_GPIO_Port, KEY_BACK_Pin) == 0)		return KEY_BACK;
	return NO_KEY;//未触发，返回0
}

void Key_Tick(void)
{
	static uint8_t Count;
	static uint8_t CurrState, PrevState;
	
	Count ++;
	if (Count >= 20)//10分频
	{
		Count = 0;//20ms进一次
		
		PrevState = CurrState;//记录
		CurrState = Key_GetState();//刷新
		
		if (CurrState== 0 && PrevState!= 0)//当前已出发，20ms前没触发，也就是确认出发
		{
			Key_Num = PrevState;//
		}
	}
}
