/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "OLED_Menu.h"
#include "LED.h"
//#include "TEST_DSP.h"
#include "ADXL345.h"
#include "ds18b20.h"
#include "my_delay.h"
#include "OLED.h"
#include "OLED_Menu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
LED_TypeDef LED_SYS;

uint8_t LED1Mode;
uint8_t LED2Mode;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t flag_500ms =0;
  static uint32_t flag_5s =0;
	if(htim->Instance==TIM2)//1MS  timer interrupt
	{
    Key_Tick();//1 MS KEY
    if(++flag_500ms > 500)        //500ms LED 
    {
      LED_SetState(&LED_SYS, LED_BLINK);
      flag_500ms =0;
    }
    if(++flag_5s > 5000)        //5s LED
    {
      ADXL345_TEMP_Calibrate(); // ADXL345 temperature calibrate . MAY SHOULD TAKE IT OUT
      flag_5s =0;
    }
		//TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
// HAL AUTO DO IT!!!
	}
}

void Display_init(){
//cartton start
  OLED_ShowStartImage(32,0,64,64,LOGO_ST);//this function are apecial,
  HAL_Delay(150);//
  OLED_ReverseArea(32, 0,64,64);OLED_Update();// color reversion
  OLED_ShowStartString(0,64-8,"press any key to enter.",OLED_6X8);//another cartoon
  OLED_Update();				//operate
  
  while(!Key_GetNum());//wait press,press any key to enter
  OLED_Clear();	OLED_Update();//clear screen

// 初始化新菜单系统
OLED_Menu_Init();
// 显示主菜单
MenuDisplay_DrawMenu(&g_menuManager);

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  ADXL345_Init(); // ADXL345 init
  HAL_TIM_Base_Start_IT(&htim2);// start TIM2, 1ms interrupt
  HAL_Delay(100); // wait for system stable
  LED_Init(&LED_SYS, LED_SYS_GPIO_Port, LED_SYS_Pin); // system status led
	OLED_Init();// OLED init
  delay_init(168);
  DS18B20_Init(); // DS18B20 init

  Display_init(); // OLED display init
  
//  MyDSP_Init();
//  arm_fir_f32_lp();
//  arm_hanning_f32(hanning_window, LENGTH_SAMPLE);
//  MyFFT_Process();
  //test();
  //MyDSP_Process();
  //arm_rfft_fast_f32_app();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // 菜单更新
    Choose_menu();  // 处理按键并更新菜单显示

    // 调试信息：显示当前选中的菜单项（可选）
    // OLED_ShowNum(0, 0, Num_Choose, 1, OLED_8X16);
    // OLED_Update();
    
    
    //test_dsp();
    
    //MyDSP_Process();//HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
