#include "LED.h"

/********************************************************************
   * @brief  : LED初始化函数
   * @param  : 其实已经GPIIO初始化了，这里是进行结构体初始化
   * @retval :
*********************************************************************/
void LED_Init(LED_TypeDef *led, GPIO_TypeDef *port, uint16_t pin)
{
    led->port = port;
    led->pin = pin;
    led->state = LED_OFF; // 默认状态为灭
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // 初始化为灭
}
/********************************************************************
   * @brief  : 此函数可改变灯的工作状态
   * @param  : 灯的状态有三种：亮灭闪
   * @retval :
*********************************************************************/
void LED_SetState(LED_TypeDef *led, LED_State state)
{
    led->state = state;
    switch (state)
    {
        case LED_OFF:
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
            break;
        case LED_ON:
            HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
            break;
        case LED_BLINK:
            HAL_GPIO_TogglePin(led->port, led->pin);
            //HAL_Delay(500); // 500ms delay for blinking
            break;
        default:
            break;
    }
}
