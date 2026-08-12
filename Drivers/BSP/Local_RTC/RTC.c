/*
 * RTC.c
 *
 *  Created on: Jul 8, 2026
 *      Author: Chanh
 */
#include "RTC.h"
#include <stdio.h>

// 声明在 main.c 中由 CubeMX 生成的 RTC 核心句柄
extern RTC_HandleTypeDef hrtc;

/**
 * @brief  初始化本地硬件 RTC
 * @retval 1: RTC 已有效，无需复位
 *         0: RTC 无效，需要等待网络 NTP 注入时间
 */
uint8_t Local_RTC_Init(void)
{
    // 在 STM32F103 中，利用备份寄存器（BKP_DR1）来记住单片机是不是第一次开机。
    // 如果纽扣电池一直有电，哪怕主电源断开，BKP_DR1 里的数据也不会丢。
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0xA5A5)
    {
        // 发现暗号 0xA5A5，说明 RTC 在纽扣电池供电下一直在默默走时，千万不要重新对时
        printf("检测到后备电池供电正常，RTC 本地时钟有效，无需复位。\r\n");
        return 1; 
    }
    else
    {
        // 没找到暗号，说明是买来第一次开机，或者纽扣电池没电了
        printf("首次开机或后备电池失效，等待网络 NTP 注入时间...\r\n");
        return 0;
    }
}

/**
 * @brief  将网络时间同步到本地硬件 RTC
 * @param  net_time: 指向 ESP_Time_t 结构体的指针，包含网络时间
 * @retval HAL 状态
 */
HAL_StatusTypeDef Local_RTC_Sync(ESP_Time_t *net_time)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef status = HAL_OK;

    __HAL_RCC_PWR_CLK_ENABLE(); // 使能电源接口时钟
    HAL_PWR_EnableBkUpAccess(); // 允许访问备份寄存器

    // 1. 搬运网络时间到 HAL 库的时间结构体
    sTime.Hours   = net_time->hour;
    sTime.Minutes = net_time->minute;
    sTime.Seconds = net_time->second;
    
    // 2. 将时分秒写入硬件寄存器（使用十进制 BIN 格式）
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return HAL_ERROR;
        goto _exit;
    }

    // 3. 搬运网络日期到 HAL 库的日期结构体
    // STM32F1 硬件年份只存 0~99，代表 2000~2099 年
    sDate.Year    = (net_time->year >= 2000) ? (net_time->year - 2000) : 0;
    sDate.Month   = net_time->month;
    sDate.Date    = net_time->day;
    
    // 4. 将年月日写入硬件寄存器
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        status = HAL_ERROR;
        goto _exit;
    }

    // 5. 【关键一步】：写入成功后，在备份寄存器留下暗号 0xA5A5
    // 这样下次单片机重启，只要纽扣电池还在，就能直接读取本地时间，不需要重新连 Wi-Fi 傻等
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xA5A5);
    
    printf(">> 本地硬件 RTC 同步成功！\r\n");
    status = HAL_OK;
    
_exit:
    HAL_PWR_DisableBkUpAccess(); // 关闭备份寄存器访问权限

    return status;
}


/**
 * @brief  获取本地硬件 RTC 的当前时间
 * @param  current_time: 指向 ESP_Time_t 结构体的指针，用于存储当前时间
 * @retval None
 */
void Local_RTC_GetTime(ESP_Time_t *current_time)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // 1. 从硬件直接读取当前的时分秒
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    
    // 2. 【避坑关键】：必须紧接着调用 GetDate！
    // 即使你不需要日期，也必须读它。这是 STM32F1 的硬件锁定机制，不读 Date 会导致时间寄存器不再刷新。
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // 3. 将硬件里读出的数字，翻译回我们业务用的结构体
    current_time->year   = sDate.Year + 2000; // 补齐四位数年份
    current_time->month  = sDate.Month;
    current_time->day    = sDate.Date;
    current_time->hour   = sTime.Hours;
    current_time->minute = sTime.Minutes;
    current_time->second = sTime.Seconds;
}

