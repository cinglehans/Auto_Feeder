/*
 * encoder.c
 *
 *  Created on: Dec 7, 2025
 *      Author: Chanh
 */

#include "EXTI.h"

#include "../KEY/key.h"
#include "../LED/led.h"


extern uint16_t pwmDuty;
extern TIM_HandleTypeDef htim2;
// extern TIM_HandleTypeDef htim3;
extern volatile FeedState_t State_Code;  // 状态机状态码
extern volatile uint8_t Feed_Flag;

uint16_t Encoder_Count;

uint16_t Encoder_Get(void)
{
	//uint16_t Temp;
	//Temp = Encoder_Count;
	//Encoder_Count = 0;
	return Encoder_Count;
}




void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	HAL_Delay(20);


	switch (GPIO_Pin)
		{
		case Key_Up_Pin:
			if ( HAL_GPIO_ReadPin(Key_Up_GPIO_Port, Key_Up_Pin) == 1 )
			{
				Feed_Flag = 3;
			}
			break;

/*
 			case Key_Re_Pin:
			if ( Key_Re == 1 )
			{
				HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_12);
			}
			break;

		case Key0_Pin:
			if ( Key0 == 0 )
			{
				if( pwmDuty >= (Min_Period+Step_Period) && pwmDuty<= Max_Period)
				{
					pwmDuty -= Step_Period;
				}
				else
				{
					pwmDuty = Min_Period;
				}
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,pwmDuty);
			}
			break;

		case Key1_Pin:
			if ( Key1 == 0 )
			{
				if( pwmDuty >= 0 && pwmDuty<= (Max_Period-Step_Period))
				{
					pwmDuty += Step_Period;
				}
				else
				{
					pwmDuty = Max_Period;
				}
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4,pwmDuty);
			}
			break;

		case encoder_A_Pin:
			if(a_pin == GPIO_PIN_RESET && b_pin == GPIO_PIN_SET)
			{
				LED0_R_TOGGLE();
				Encoder_Count --;

			}
			else if(a_pin == GPIO_PIN_SET && b_pin == GPIO_PIN_RESET)
			{
				LED0_R_TOGGLE();
				Encoder_Count --;
			}
			break;

		case encoder_B_Pin:
			if (b_pin == GPIO_PIN_RESET && a_pin == GPIO_PIN_SET)
			{
				LED1_G_TOGGLE();
				Encoder_Count ++;
			}
			break;
 *
 */
		}
}
