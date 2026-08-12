/*
 * Hall_Sensor.h
 *
 *  Created on: Jun 22, 2026
 *      Author: Chanh
 */

#ifndef BSP_HALL_SENSOR_HALL_SENSOR_H_
#define BSP_HALL_SENSOR_HALL_SENSOR_H_

#include "main.h"

//密封霍尔传感器引脚定义
#define Hall_Seal_Pin		Seal_Detect_Pin
#define Hall_Seal_Port		Seal_Detect_GPIO_Port

//放粮霍尔传感器引脚定义
#define Hall_Feed_Pin		Feed_Detect_Pin
#define Hall_Feed_Port	    Feed_Detect_GPIO_Port

#define Hall_Seal_Read()		HAL_GPIO_ReadPin(Hall_Seal_Port, Hall_Seal_Pin)
#define Hall_Feed_Read()		HAL_GPIO_ReadPin(Hall_Feed_Port, Hall_Feed_Pin)

//霍尔状态
#define Hall_Triggered		GPIO_PIN_RESET
#define Hall_Released		GPIO_PIN_SET


uint8_t Hall_Sensor_Read(GPIO_TypeDef* Port, uint16_t Pin, GPIO_PinState expectedState);
uint8_t Hall_Seal_Check(void);
uint8_t Hall_Feed_Check(void);


#endif /* BSP_HALL_SENSOR_HALL_SENSOR_H_ */
