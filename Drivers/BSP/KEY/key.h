#ifndef BSP_KEY_KEY_H_
#define BSP_KEY_KEY_H_

#include "main.h"

#define Key_Up		    HAL_GPIO_ReadPin(Key_Up_GPIO_Port, Key_Up_Pin)
#define Key0			HAL_GPIO_ReadPin(Key0_GPIO_Port, Key0_Pin)
#define Key1			HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin)

#define Keywk_press		1
#define Key0_press		2

uint8_t Key_Scan(uint8_t mode);

#endif
