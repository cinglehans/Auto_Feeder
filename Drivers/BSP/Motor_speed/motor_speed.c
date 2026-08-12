/*
 * motor_speed.c
 *
 *  Created on: Feb 2, 2026
 *      Author: Chanh
 */


#include "motor_speed.h"
#include "math.h"
#include "string.h"

static MotorSpeed_Handle_t* motor_handle = NULL;
/**
*@breif 初始化电机速度测量模块
*@param hmotor 指向电机速度测量句柄的指针
*@param unit 速度单位
*/
void MotorSpeed_Init(MotorSpeed_Handle_t* handle, SpeedUnit_t unit)
{
    if(handle == NULL) return;
    motor_handle = handle;

    motor_handle->last_capture_value = 0;
    motor_handle->capture_period = 0;
    motor_handle->pulse_count = 0;
    motor_handle->last_pulse_time = 0;

    motor_handle->current_speed = 0.0f;
    motor_handle->filtered_speed = 0.0f;
    motor_handle->average_speed = 0.0f;
    motor_handle->max_speed = 0.0f;
    motor_handle->min_speed = 1000000.0f; // Set to a high initial value

    motor_handle->speed_unit = unit;
    motor_handle->motor_state = MOTOR_STOPPED;
    motor_handle->measurement_valid = 0;
    motor_handle->update_time = HAL_GetTick();

    memset(motor_handle->speed_buffer, 0, sizeof(motor_handle->speed_buffer));
    motor_handle->buffer_index = 0;

//    g_motor_speed_handle = motor_handle;
    //启动编码器接口
    HAL_TIM_Encoder_Start(motor_handle->htim, TIM_CHANNEL_2);
}

/**
*@brief 启动速度测量
*@param handle 速度测量句柄
 */
void MotorSpeed_Start(MotorSpeed_Handle_t* handle)
{
    if(handle == NULL) return;
    
    MotorSpeed_Init(handle, handle->speed_unit);  //重新初始化
    HAL_TIM_IC_Start_IT(handle->htim, TIM_CHANNEL_1); //启动输入捕获中断

    handle->motor_state = MOTOR_RUNNING;  //设置电机状态为运行
    handle->update_time = HAL_GetTick();  //记录启动时间
}

/**
*@brief 停止速度测量
*@param handle 速度测量句柄
 */
 void MotorSpeed_Stop(MotorSpeed_Handle_t* handle)
 {
    if(handle == NULL) return;
    
    HAL_TIM_IC_Stop_IT(handle->htim, TIM_CHANNEL_1); //停止输入捕获中断

    handle->motor_state = MOTOR_STOPPED;  //设置电机状态为停止
 }

 /**
*@brief 根据捕获周期计算速度
*@param period 捕获周期（即定时器计数值差）
*@param unit 速度单位
*@return 计算得到的速度值
 */
 static float MotorSpeed_Calculated(uint32_t pulse_count, float time_interval,SpeedUnit_t unit)
 {
    if(time_interval == 0) return 0.0f; //避免除以零

    //计算每秒脉冲数
    float pulses_freq = (float)pulse_count / (float)time_interval;

    //计算转速（RPM）
    float rpm = (pulses_freq / motor_PPR) * 60.0f;

    switch(unit)
    {
        case SPEED_RPM:
            return rpm;
        case SPEED_RPS:
            return rpm / 60.0f;
        case SPEED_DPS:
            return (rpm * 360.0f) / 60.0f;
        default:
            return rpm; //默认返回RPM
    }
 }

/**
*@brief 速度测量更新函数，应在定时器中断或主循环中定期调用
*@param handle:速度测量句柄
*/
void MotorSpeed_Update(MotorSpeed_Handle_t* handle)
{
    if(handle == NULL) return;

    uint32_t current_time = HAL_GetTick();
    uint32_t time_diff = current_time - handle->update_time;
    //检查是否超时无脉冲，用于判断电机是否停止
    if(time_diff >= speed_update_interval_ms*3)
    {
        if(handle->motor_state != MOTOR_STOPPED)
        {
            handle->motor_state = MOTOR_STOPPED;
            handle->current_speed = 0.0f;
            handle->filtered_speed = 0.0f;
            handle->measurement_valid = 0;
        }
        return;
    }

    if(time_diff >= speed_update_interval_ms)
    {
        float time_interval_ms = (float)time_diff / 1000.0f; //转换为秒
        //计算当前速度
        float current_speed = MotorSpeed_Calculated(handle->pulse_count, time_interval_ms, handle->speed_unit);

        handle->current_speed = current_speed;
        handle->measurement_valid = 1;

         //更新最大最小速度
        if(handle->current_speed > handle->max_speed)
            handle->max_speed = handle->current_speed;
        if(handle->current_speed < handle->min_speed && current_speed > 0.0f)
            handle->min_speed = handle->current_speed;

        //更新速度缓冲区
        handle->speed_buffer[handle->buffer_index] = current_speed;
        handle->buffer_index = (handle->buffer_index + 1) % speed_filter_samples;

        //计算滤波速度
        float sum_speed = 0.0f;
        for(uint8_t i = 0; i < speed_filter_samples; i++)
        {
            sum_speed += handle->speed_buffer[i];
        }
        handle->filtered_speed = sum_speed / speed_filter_samples;

       

        //重置脉冲计数和更新时间
        handle->pulse_count = 0;
        handle->update_time = current_time;
    }
}

/**
  * @brief  获取当前速度
  * @param  handle: 速度测量句柄
  * @return 当前速度值
  */
float MotorSpeed_GetSpeed(MotorSpeed_Handle_t* handle)
{
    if (handle == NULL) return 0.0f;
    return handle->current_speed;
}

/**
  * @brief  获取滤波后的速度
  * @param  handle: 速度测量句柄
  * @return 滤波后的速度值
  */
float MotorSpeed_GetFilteredSpeed(MotorSpeed_Handle_t* handle)
{
    if (handle == NULL) return 0.0f;
    return handle->filtered_speed;
}

/**
  * @brief  获取电机状态
  * @param  handle: 速度测量句柄
  * @return 电机状态
  */
MotorState_t MotorSpeed_GetState(MotorSpeed_Handle_t* handle)
{
    if (handle == NULL) return MOTOR_STOPPED;
    return handle->motor_state;
}

/**
  * @brief  重置测量数据
  * @param  handle: 速度测量句柄
  */
void MotorSpeed_Reset(MotorSpeed_Handle_t* handle)
{
    if (handle == NULL) return;
    
    handle->pulse_count = 0;
    handle->last_capture_value = 0;
    handle->capture_period = 0;
    handle->current_speed = 0.0f;
    handle->max_speed = 0.0f;
    handle->min_speed = 100000.0f;
    handle->average_speed = 0.0f;
    handle->measurement_valid = 0;
    handle->update_time = HAL_GetTick();
    
    memset(handle->speed_buffer, 0, sizeof(handle->speed_buffer));
    handle->buffer_index = 0;
    
//    printf("Motor Speed Measurement Reset\n");
}

/**
  * @brief  设置速度单位
  * @param  handle: 速度测量句柄
  * @param  unit: 新的速度单位
  */
void MotorSpeed_SetUnit(MotorSpeed_Handle_t* handle, SpeedUnit_t unit)
{
    if (handle == NULL) return;
    
    // 转换现有速度值到新单位
    if (handle->measurement_valid)
    {
        handle->current_speed = MotorSpeed_ConvertUnit(handle->current_speed, handle->speed_unit, unit);
        handle->filtered_speed = MotorSpeed_ConvertUnit(handle->filtered_speed, handle->speed_unit, unit);
        handle->max_speed = MotorSpeed_ConvertUnit(handle->max_speed, handle->speed_unit, unit);
        handle->min_speed = MotorSpeed_ConvertUnit(handle->min_speed, handle->speed_unit, unit);
        handle->average_speed = MotorSpeed_ConvertUnit(handle->average_speed, handle->speed_unit, unit);
        
        // 转换滤波缓冲区
        for (uint8_t i = 0; i < speed_filter_samples; i++)
        {
            handle->speed_buffer[i] = MotorSpeed_ConvertUnit(handle->speed_buffer[i], handle->speed_unit, unit);
        }
    }
    
    handle->speed_unit = unit;
//    printf("Speed unit changed to: %d\n", unit);
}

/**
  * @brief  速度单位转换
  * @param  speed: 原始速度值
  * @param  from: 原始单位
  * @param  to: 目标单位
  * @return 转换后的速度值
  */
float MotorSpeed_ConvertUnit(float speed, SpeedUnit_t from, SpeedUnit_t to)
{
    if (from == to) return speed;
    
    // 首先转换为脉冲频率（Hz）
    float pulse_freq;
    switch (from)
    {
        case SPEED_DPS:
            pulse_freq = speed;
            break;
        case SPEED_RPS:
            pulse_freq = speed * motor_PPR;
            break;
        case SPEED_RPM:
            pulse_freq = (speed * motor_PPR) / 60.0f;
            break;
        default:
            return 0.0f;
    }
    
    // 然后转换为目标单位
    switch (to)
    {
        case SPEED_DPS:
            return pulse_freq;
        case SPEED_RPS:
            return pulse_freq / motor_PPR;
        case SPEED_RPM:
            return (pulse_freq * 60.0f) / motor_PPR;
        default:
            return 0.0f;
    }
}


/* 中断回调函数 */

/**
  * @brief  输入捕获中断回调函数
  * @param  htim: 定时器句柄
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (motor_handle == NULL) return;
    
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        // 获取当前捕获值
        uint32_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        
        // 计算周期（处理计数器溢出）
        if (motor_handle->last_capture_value != 0)
        {
            uint32_t period;
            if (current_capture > motor_handle->last_capture_value)
            {
                period = current_capture - motor_handle->last_capture_value;
            }
            else
            {
                // 计数器溢出
                period = (timer_max_count - motor_handle->last_capture_value) + current_capture;
            }
            
            motor_handle->capture_period = period;
            
            // T法测速：根据周期计算瞬时速度
//            float instant_speed = Calculate_Speed_From_Period(period, motor_handle->speed_unit);
//            motor_handle->current_speed = instant_speed;
        }
        
        // 更新上次捕获值
        motor_handle->last_capture_value = current_capture;
        
        // 更新脉冲计数（用于M法测速）
        motor_handle->pulse_count++;
        motor_handle->last_pulse_time = HAL_GetTick();
        
        // 更新状态
        if (motor_handle->motor_state == MOTOR_STOPPED)
        {
            motor_handle->motor_state = MOTOR_RUNNING;
        }
    }
}
