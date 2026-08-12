/* USER CODE BEGIN Header */
/**
 ****************************************************************pp**************
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
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../BSP/LED/led.h"
//#include "../../BSP/KEY/key.h"
#include "../../BSP/OLED/OLED.h"
#include "../../Drivers/BSP/EXTI/EXTI.h"
#include "../../Drivers/BSP/Motor_Control/Motor_Control.h"
#include "../../Drivers/BSP/Feed_Ctrl/Feed_Ctrl.h"
#include "../../Drivers/BSP/Hall_Sensor/Hall_Sensor.h"
#include "../../Drivers/BSP/Local_RTC/RTC.h"
#include "../../Drivers/BSP/WIFI/WIFI.h"
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
// PWM初始占空�????
uint16_t pwmDuty = 25;

//喂食标志
volatile uint8_t Feed_Flag = 0;
volatile uint8_t Feed_Times = 0; // 喂食次数，由网络指令触发


volatile FeedState_t State_Code = State_Init_Seal; // 状�?�机编码
const char *State[] =
		{ "Init", "Seal", "ToFeed", "HoldFeed", "ToSeal", "Error" };

ESP_Time_t live_time;   //整个系统共享的当前时间结构体
ESP_Status_t wifi_status;   //wifi状�?�机返回状�??
uint8_t is_synced = 0;   //标记今天是否已经同步过时�????

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

	//OLED显示初始�??
	oled_init();
	oled_show_string(0, 0, "Seal:", 12);
	oled_show_string(0, 12, "Feed:", 12);
	oled_show_string(0, 24, "State:", 12);
	oled_show_string(0, 36, "Flag:", 12);
	oled_show_string(0, 48, "LState: ", 12);
	oled_show_string(60, 0, "O1:", 12);
	oled_show_string(60, 12, "O2:", 12);
	oled_refresh_gram(); /* 更新显示到OLED */

	if (Local_RTC_Init() == 1) {
		is_synced = 1;   // 标记已经同步过时�???
	}

	ESP_Time_t current_time;
	HAL_Delay(1000); // 等待系统稳定
	//初始化网络串口，触发�??次异步对时流�??
	ESP_Network_Init(&huart2, "Hz.C", "cinv7401", "9b78ec73fc3ca4eb13120efe34302248", "iFagZXZ72002");
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

    WIFI_Rx_Parse();   //解析串口接收缓冲区，提取AT指令返回的关键信息
		ESP_Status_t net_status = ESP_Network_Task(&current_time);	//联网状态机轮询，AT指令连接WiFi，连接云服务器，实时获取命令，获取网络时间；

		//  Local_RTC_GetTime(&live_time);   // 获取本地RTC时间
		//  if (live_time.hour == 8 && live_time.minute == 0)
		//  {
		//  	// 每天早上8点触发一次喂食操�???
		//  	static uint8_t fed_morning = 0; // 标记今天早上是否已经喂过
		//  	if (fed_morning == 0) {
		//  		Feed_Flag = 1; // 执行喂食操作
		//  		fed_morning = 1; // 标记今天早上已经喂过
		//  	}
		//  }

		/*
		 喂食器状态机流程轮询
		 */
		Feed_Control(); // 喂食器状态机控制

		/*
			@brief  实时读取传感器状态并显示在OLED
		*/
		uint8_t Seal_State = Hall_Seal_Read(); // 密封状�??
		uint8_t Feed_State = Hall_Feed_Read(); // 出粮状�??
		uint8_t O1 = HAL_GPIO_ReadPin(IN1_Port, IN1_Pin); // 读取O1状�??
		uint8_t O2 = HAL_GPIO_ReadPin(IN2_Port, IN2_Pin); // 读取O2状�??
		 if(net_status == ESP_OK)
		 {
		 oled_show_num(30, 0, Seal_State, 4, 12); /*显示Hall_Seal传感器读取�??*/
		 oled_show_num(30, 12, Feed_State, 4, 12); /*显示Hall_Feed传感器读取�??*/
		 oled_show_num(30, 36, Feed_Flag, 4, 12); /*显示喂食标志*/
		 oled_show_num(90, 12, O1, 4, 12); /*显示O1状�??*/
		 oled_show_num(90, 0, O2, 4, 12); /*显示O2状�??*/
		 oled_show_string(35, 24, State[State_Code], 12); /*显示状�?�机状�??*/
		 oled_refresh_gram(); /* 更新显示到OLED */
		 }
//		printf("采样值为%d\r\n",adcdata);
//		temp = (float)adcdata/4096*3.3;
//		printf("实际电压值为%f\r\n",temp);
//		 adcdata = temp;
//		 oled_show_num(54,52,adcdata,1,12);	/*显示电压整数部分*/
//		 oled_show_string(66,52,".",12);		/*显示小数*/
//		 temp -= adcdata;
//		 temp *= 1000;
//		 oled_show_num(78, 52, temp, 3, 12);    /*显示电压小数部分*/
//		 oled_refresh_gram(); /*更新显示到OLED */


	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
	while (1) {
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
