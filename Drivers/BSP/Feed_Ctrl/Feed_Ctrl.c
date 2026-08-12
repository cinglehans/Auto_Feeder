/*
 * Feed_Ctrl.c
 *
 *  Created on: Jun 23, 2026
 *      Author: Chanh
 */
#include "Feed_Ctrl.h"
extern volatile Motor_State_t M_State;  //电机状态
extern volatile FeedState_t State_Code;  // 状态机状态码
extern volatile uint8_t Feed_Flag;   // 喂食标志位
extern volatile uint8_t Feed_Times; // 喂食次数，由网络指令触发

extern TIM_HandleTypeDef htim3;      // 定时器句柄
extern const char *State[]; // 状态机状态字符串数组
static uint8_t Init_Retry_Count = 0; // 初始化重试计数器
static uint32_t start_time; // 用于记录状态切换的起始时间
static uint8_t Feed_Count = 0;

void Feed_Control(void)
{
    static FeedState_t last_state = 0; // 上一次的状态码
    if (State_Code != last_state)
    {
    	oled_show_string(60, 48, State[last_state], 12);
    	last_state = State_Code;
    	oled_refresh_gram(); /* 更新显示到OLED */
    }
    switch(State_Code)
    {
        case State_Init_Seal:
            Feed_Flag = 0; // 重置喂食标志位
            uint8_t init_result = Init_Seal();
            if(init_result == 1)
            {
                Init_Retry_Count = 0; // 重置初始化重试计数器
                State_Code = State_Seal;
                printf("初始化成功，进入密封状态\r\n");
            }
            else if(init_result == 0)
            {
                Init_Retry_Count++;
                if(Init_Retry_Count >= MAX_INIT_RETRIES)
                {
                    printf("初始化失败，达到最大重试次数\r\n");
                    State_Code = State_Error; // 进入错误状态
                    // 可以在这里添加错误处理逻辑，例如进入安全模式或报警
                }
            }
            break;

        case State_Seal:
            if((Feed_Count == 0) && (M_State != Brake))
            {
                Motor_Brake(); // 确保电机停止
            }
            // 等待喂食命令
            if(Feed_Flag == 1)
            {
                Feed_Count = Feed_Times; // 设置喂食次数
                Feed_Flag = 0;
            }
            
            if (Feed_Count > 0)
            {
                Feed_Count --;
                State_Code = State_To_Feed;
                start_time = HAL_GetTick(); // 记录每次进入出粮状态的起始时间
            }
            else if (Feed_Count == 0)
            {

            }

            break;

        case State_To_Feed:
            // 转向出粮状态
            if(M_State == Brake)
            {
                Motor_CW();
            }

            if(Hall_Feed_Check())
            {
                Motor_Brake();
                State_Code = State_Hold_Feed;
                Timer_Delay_ms(Feed_Hold_Time_ms);
            }
            else if(HAL_GetTick()-start_time > Timeout_ms)
            {
                // HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET); // 蜂鸣器发声提示
                Motor_Brake();
                printf("放粮超时，无法找到出粮状态\r\n");
                State_Code = State_To_Seal; // 超时后返回密封状态
                start_time = HAL_GetTick(); // 记录每次进入转回密封状态的起始时间
                Feed_Flag = 0; // 超时后重置喂食标志位
            }
            break;

        case State_Hold_Feed:
            // 保持出粮状态
            Motor_Brake();
            // printf("保持出粮状态\r\n");
            break;

        case State_To_Seal:
            // 转回密封状态
            if(M_State == Brake)
            {
                Motor_CW();
            }
            if(Hall_Seal_Check())
            {
                // Motor_Brake();
                State_Code = State_Seal;
                printf("已回到密封状态\r\n");
            }
            else if(HAL_GetTick() - start_time > Timeout_ms)
            {
                Motor_Brake();
                printf("转回密封状态超时\r\n");
                State_Code = State_Init_Seal; // 超时后重新初始化
                Feed_Flag = 0; // 超时后重置喂食标志位
                return;
            }
            break;
        case State_Error:
            // 错误状态，等待人工干预
            Motor_Brake(); // 确保电机停止
            Feed_Flag = 0; // 异常状态重置喂食标志位
            printf("系统处于错误状态，请人工干预\r\n");
            break;
    }
}

/*
 * 状态机：初始状态，密封状态；放粮状态
 * 初始状态：开始搜寻密封位置；
 * 密封状态：收到放粮命令，状态变为放粮状态；
 */
uint8_t Init_Seal(void)
{

    if(M_State == Brake)
    {
        if(Hall_Seal_Check())
        {
            Motor_Brake();
            printf("已在密封状态\r\n");
            return 1;
        }
        else
        {
            start_time = HAL_GetTick(); // 记录每次初始化操作的起始时间
            Motor_CW(); // 正转寻找密封状态
            return -1; // 表示正在初始化
        }
    }

    //电机已启动，检查是否到达密封状态或超时
    if(Hall_Seal_Check())
    {
        Motor_Brake();
        printf("已在密封状态\r\n");
        return 1;
    }
    else if(HAL_GetTick() - start_time > Timeout_ms)
    {
        Motor_Brake();
        printf("初始化超时，无法找到密封状态\r\n");
        return 0; // 表示初始化失败
    }
    return -1;
}


void Timer_Delay_ms(uint16_t delay)
{
    HAL_TIM_Base_Stop_IT(&htim3);  // 停止定时器中断
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);  // 清除更新中断标志
    __HAL_TIM_SET_AUTORELOAD(&htim3, 2*delay - 1);  // 设置自动重装载值
    __HAL_TIM_SET_COUNTER(&htim3, 0);  // 将计数器清零
    HAL_TIM_Base_Start_IT(&htim3);        // 启动定时器
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		// 计时器中断处理逻辑
		HAL_TIM_Base_Stop_IT(&htim3);  // 停止计时器中断
		State_Code = State_To_Seal; // 超时后返回密封状态
        start_time = HAL_GetTick(); // 记录每次进入转回密封状态的起始时间
	}
}

