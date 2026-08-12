/*
 * RTC.h
 *
 *  Created on: Jul 8, 2026
 *      Author: Chanh
 */

#ifndef BSP_LOCAL_RTC_RTC_H_
#define BSP_LOCAL_RTC_RTC_H_

#include "stm32f1xx_hal.h"
#include "../../Drivers/BSP/WIFI/WIFI.h"  // 引入网络驱动头文件，复用里面的 ESP_Time_t 结构体

/**
 * @brief  初始化本地 RTC（主要是检查备份区域，防止重复初始化导致时间复位）
 * @return uint8_t 0: 首次初始化; 1: 靠纽扣电池维持着，无需复位时间
 */
uint8_t Local_RTC_Init(void);

/**
 * @brief  将网络获取到的标准时间同步写入 STM32 内部 RTC 硬件
 * @param  net_time: 指向已填充网络时间的结构体指针
 * @return HAL_StatusTypeDef HAL状态（HAL_OK表示成功）
 */
HAL_StatusTypeDef Local_RTC_Sync(ESP_Time_t *net_time);

/**
 * @brief  从本地 RTC 硬件中读取当前精确时间
 * @param  current_time: 存放读取结果的结构体指针
 */
void Local_RTC_GetTime(ESP_Time_t *current_time);

#endif /* BSP_LOCAL_RTC_RTC_H_ */
