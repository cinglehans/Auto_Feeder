/*
 * LED.h
 *
 *  Created on: Dec 7, 2025
 *      Author: Chanh
 */

#ifndef BSP_LED_H_
#define BSP_LED_H_

#define LED0_R(x)	do{x?\
						HAL_GPIO_WritePin(LED0_R_GPIO_Port,LED0_R_Pin,GPIO_PIN_SET):\
						HAL_GPIO_WritePin(LED0_R_GPIO_Port,LED0_R_Pin,GPIO_PIN_RESET);\
						}while(0)

#define LED1_G(x)	do{x?\
						HAL_GPIO_WritePin(LED1_G_GPIO_Port,LED1_G_Pin,GPIO_PIN_SET):\
						HAL_GPIO_WritePin(LED1_G_GPIO_Port,LED1_G_Pin,GPIO_PIN_RESET);\
						}while(0)

#define LED0_R_TOGGLE()	do{HAL_GPIO_TogglePin(LED0_R_GPIO_Port,LED0_R_Pin);}while(0)
#define LED1_G_TOGGLE()	do{HAL_GPIO_TogglePin(LED1_G_GPIO_Port,LED1_G_Pin);}while(0)

#endif /* BSP_LED_H_ */
