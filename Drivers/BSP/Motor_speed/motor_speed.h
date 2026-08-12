/*
 * motor_speed.h
 *
 *  Created on: Feb 2, 2026
 *      Author: Chanh
 */

#ifndef BSP_MOTOR_SPEED_MOTOR_SPEED_H_
#define BSP_MOTOR_SPEED_MOTOR_SPEED_H_

#include "main.h"

#define motor_PPR  18  // Pulses Per Revolution
#define timer_freq_Hz  1000000  // Timer frequency 1MHz
#define timer_max_count  65535  // 16-bit timer max count
#define speed_update_interval_ms  100  // Speed update interval in milliseconds
#define speed_filter_samples    5  // Number of samples for speed filtering

//速度单位
typedef enum {
    SPEED_RPM,      // Revolutions Per Minute
    SPEED_RPS,      // Revolutions Per Second
    SPEED_DPS       // Degrees Per Second
} SpeedUnit_t;


//电机状态
typedef enum {
    MOTOR_STOPPED,
    MOTOR_RUNNING,
    MOTOR_OVERSPEED
} MotorState_t;

//速度测量结构体
typedef struct {
    TIM_HandleTypeDef* htim;  // Timer handle for encoder interface
    
    //测量数据
    volatile uint32_t last_capture_value; // Last captured timer value
    volatile uint32_t capture_period; // captured period between two readings
    volatile uint32_t pulse_count;    // Total pulse count
    volatile uint32_t last_pulse_time; // Timestamp of last pulse

    //计算结果
    float current_speed;    // Current speed
    float filtered_speed;   // Filtered speed
    float average_speed;    // Average speed
    float max_speed;        // Maximum speed recorded
    float min_speed;        // Minimum speed recorded
    
    //状态
    MotorState_t motor_state; // Motor state
    SpeedUnit_t speed_unit;   // Speed unit
    uint8_t measurement_valid; // Flag indicating if the measurement is valid
    uint32_t update_time;   //timestamp of last speed update

    //滤波缓冲区
    float speed_buffer[speed_filter_samples]; // Buffer for speed filtering
    uint8_t buffer_index;     // Current index in speed buffer

} MotorSpeed_Handle_t;


void MotorSpeed_Init(MotorSpeed_Handle_t* handle, SpeedUnit_t unit);
void MotorSpeed_Start(MotorSpeed_Handle_t* handle);
void MotorSpeed_Stop(MotorSpeed_Handle_t* handle);
float MotorSpeed_GetSpeed(MotorSpeed_Handle_t* handle);
float MotorSpeed_GetFilteredSpeed(MotorSpeed_Handle_t* handle);
MotorState_t MotorSpeed_GetState(MotorSpeed_Handle_t* handle);
void MotorSpeed_Update(MotorSpeed_Handle_t* handle);
void MotorSpeed_Reset(MotorSpeed_Handle_t* handle);
void MotorSpeed_SetUnit(MotorSpeed_Handle_t* handle, SpeedUnit_t unit);
float MotorSpeed_ConvertUnit(float speed, SpeedUnit_t from, SpeedUnit_t to);

#endif /* BSP_MOTOR_SPEED_MOTOR_SPEED_H_ */
