 /*
 * Motor_Control.c
 *
 *  Created on: Jun 17, 2026
 *      Author: Chanh
 */

 
 #include "Motor_Control.h"
 #include "../../BSP/LED/led.h"
 

/* 电机控制宏（TB6612驱动板）
 *
 */
volatile Motor_State_t M_State = Brake;

//电机驱动初始化
void Motor_Enable(void)
{
    HAL_GPIO_WritePin(STBY_Port,STBY_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(PWMA_GPIO_Port,PWMA_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN1_Port,IN1_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_Port,IN2_Pin,GPIO_PIN_RESET);
}

void Motor_Disable(void)
{
    HAL_GPIO_WritePin(STBY_Port,STBY_Pin,GPIO_PIN_RESET);
}

void Motor_Init(void)
{
	Motor_Enable();
	Motor_Brake();
}Motor_State_t;

//电机正转
void Motor_CW(void)
{
    M_State = CW;
    Motor_Enable();
    HAL_GPIO_WritePin(IN1_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_Port, IN2_Pin, GPIO_PIN_RESET);
    LED0_R(0);
    LED1_G(1);
}


//电机反转
void Motor_CCW(void)
{
    M_State = CCW;
    Motor_Enable();
    HAL_GPIO_WritePin(IN1_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_Port, IN2_Pin, GPIO_PIN_SET);
    LED0_R(0);
    LED1_G(1);
}

//电机刹车
void Motor_Brake(void)
{
    M_State = Brake;
    Motor_Enable();
    HAL_GPIO_WritePin(IN1_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_Port, IN2_Pin, GPIO_PIN_SET);
    LED0_R(1);
    LED1_G(0);
}


/*规定转盘槽口与出粮口中心重合为0°
 *  状态定义：初始状态；密封状态；放粮状态
 * 初始状态：转盘位置未知，需正反转装盘寻找密封状态
 * 密封状态：转盘槽口旋转至180°，密封霍尔传感器触发，磁石位于0°位置
 * 放粮状态：转盘槽口旋转至0°，放粮霍尔传感器触发，磁石位于180°位置
 */


// void Feed_Once(void)
// {
//     // 1. 确保当前在密封状态
//     Init_Seal();

//     // 2. 转向出粮位置（传感器A）
//     Motor_CW();
//     uint32_t start_time = HAL_GetTick();
//     while(!Hall_Feed_Check())
//     {
//         if(HAL_GetTick() - start_time > Timeout_ms)
//         {
//             Motor_Brake();
//             printf("放粮超时，无法找到出粮状态\r\n");
//             return;
//         }
//     }
//     Motor_Brake();
//     printf("已到达出粮状态\r\n");

//     // 3. 保持出粮
//     HAL_Delay(Feed_Hold_Time_ms);  // 落粮时间

//     // 4. 转回密封位置（传感器B）
//     Motor_CW();
//     start_time = HAL_GetTick();
//     while(!Hall_Seal_Check())
//     {
//         if(HAL_GetTick() - start_time > Timeout_ms)
//         {
//             Motor_Brake();
//             printf("转回密封状态超时\r\n");
//             return;
//         }
//     }
//     Motor_Brake();
// }
