/*
 * encoder.h
 *
 *  Created on: Dec 7, 2025
 *      Author: Chanh
 */

#ifndef BSP_EXTI_EXTI_H_
#define BSP_EXTI_EXTI_H_

#include "main.h"
#include "../../BSP/Feed_Ctrl/Feed_Ctrl.h"

#define a_pin	HAL_GPIO_ReadPin(encoder_A_GPIO_Port,encoder_A_Pin)
#define b_pin	HAL_GPIO_ReadPin(encoder_B_GPIO_Port,encoder_B_Pin)

#define	Max_Period	50
#define	Min_Preriod	1
#define Step_Period	2

uint16_t Encoder_Get(void);

#endif /* BSP_EXTI_EXTI_H_ */
