/*
 * Feed_Ctrl.h
 *
 *  Created on: Jun 23, 2026
 *      Author: Chanh
 */

#ifndef BSP_FEED_CTRL_FEED_CTRL_H_
#define BSP_FEED_CTRL_FEED_CTRL_H_

#include "main.h"
#include "../../BSP/Motor_Control/Motor_Control.h"
#include "../../BSP/OLED/oled.h"

//电机控制可调流程参数
#define Speed_Ramp_Step     20    //缓启动每步占空比增量
#define Ramp_Delay_ms       50    //缓启动每步延时ms
#define Feed_Hold_Time_ms   3000  //放粮保持时间ms
#define Timeout_ms          5000  //安全超时时间ms

#define MAX_INIT_RETRIES 3 // 最大初始化重试次数


typedef enum {
    State_Init_Seal,    // 上电后初始化寻找密封状态
    State_Seal,         // 密封状态稳定
    State_To_Feed,      // 转向出粮状态
    State_Hold_Feed,    // 保持出粮状态
    State_To_Seal,     // 转回密封状态
    State_Error         // 错误状态，等待人工干预
} FeedState_t;

void Feed_Control(void);
uint8_t Init_Seal(void);
void Timer_Delay_ms(uint16_t delay_ms);

#endif /* BSP_FEED_CTRL_FEED_CTRL_H_ */
