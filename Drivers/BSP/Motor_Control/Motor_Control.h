/*
 * Motor_Control.h
 *
 *  Created on: Jun 17, 2026
 *      Author: Chanh
 */

#ifndef BSP_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define BSP_MOTOR_CONTROL_MOTOR_CONTROL_H_

#include "main.h"
#include "../../BSP/Hall_Sensor/Hall_Sensor.h"

#define IN1_Pin		    Motor_CW_Pin	//正转输出控制引脚
#define	IN1_Port	    Motor_CW_GPIO_Port

#define IN2_Pin		    Motor_CCW_Pin	//反转输出控制引脚
#define	IN2_Port	    Motor_CCW_GPIO_Port

#define STBY_Pin		Motor_STBY_Pin	//使能输出控制引脚
#define	STBY_Port	    Motor_STBY_GPIO_Port

#define TRIGGERED		GPIO_PIN_RESET   // 假设模块磁铁靠近时输出低
#define RELEASED		GPIO_PIN_SET

typedef enum
{
    CW,
    CCW,
    Brake
}Motor_State_t;

//电机控制可调流程参数
#define Speed_Ramp_Step     20    //缓启动每步占空比增量
#define Ramp_Delay_ms       50    //缓启动每步延时ms
#define Feed_Hold_Time_ms   3000  //放粮保持时间ms
#define Timeout_ms          5000  //安全超时时间ms

void Motor_Init(void);
void Motor_CW(void);
void Motor_CCW(void);
void Motor_Brake(void);
void Motor_Enable(void);
void Motor_Disable(void);


#endif /* BSP_MOTOR_CONTROL_MOTOR_CONTROL_H_ */
