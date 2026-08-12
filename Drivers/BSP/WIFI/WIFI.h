/*
 * WIFI.h
 *
 *  Created on: Jul 8, 2026
 *      Author: Chanh
 */

#ifndef BSP_WIFI_WIFI_H_
#define BSP_WIFI_WIFI_H_

// 引入 STM32 HAL 库核心头文件，用于串口操作和获取系统时间戳
#include "stm32f1xx_hal.h"
#include "main.h"

/**
 * @brief ESP8266 异步执行状态及错误码枚举
 */
typedef enum {
    ESP_OK = 0,         // 对时成功！且MQTT顺利在线，系统一切正常
    ESP_BUSY,           // 模块正在忙碌中（走状态机流程或等待硬件回复中）
    ESP_ERR_TIMEOUT,    // 某一步骤响应超时，硬件无应答
    ESP_ERR_NO_MODULE,  // 模块完全死机或未连接（AT测试无响应）
    ESP_ERR_WIFI,       // Wi-Fi 连接失败（密码错或信号差）
    ESP_ERR_PARSE       // NTP字符串解析失败
} ESP_Status_t;

/**
 * @brief 北京时间结构体
 */
typedef struct {
    uint16_t year;      // 年
    uint8_t  month;     // 月
    uint8_t  day;       // 日
    uint8_t  hour;      // 时
    uint8_t  minute;    // 分
    uint8_t  second;    // 秒
} ESP_Time_t;

/* ==================================================================== */
/*                       对外公开的非阻塞接口                           */
/* ==================================================================== */

/*
@brief  解析接收缓冲区，提取标志并清空缓冲（在主循环中调用）
*/
void WIFI_Rx_Parse(void);


/**
 * @brief  初始化：绑定串口
 * @param  huart: 连接 ESP8266 的串口句柄指针（如 &huart2）
 */
void ESP_Network_Init(UART_HandleTypeDef *huart, const char* ssid, const char* pwd, const char* uid, const char* topic);

/**
 * @brief  启动对时流水线：仅负责配置参数并触发状态机，瞬间执行完毕，不阻塞
 * @param  ssid: Wi-Fi 名称字符串
 * @param  pwd:  Wi-Fi 密码字符串
 */
void ESP_Reset_StateMachine(void);

/*
@brief 硬件复位ESP8266模块
*/

/**
 * @brief  非阻塞核心任务引擎：需要在 main.c 的 while(1) 里被疯狂调用
 * @param  out_time: 接收最终时间的结构体指针
 * @return ESP_Status_t 返回当前网络状态（返回 ESP_BUSY 表示还在走流程，返回 ESP_OK 表示对时完成）
 */
ESP_Status_t ESP_Network_Task(ESP_Time_t *net_time);

/**
 * @brief  中断数据传送带：在串口接收中断回调函数里调用，塞入原生字节
 * @param  byte: 中断收到的单字节数据
 */
void ESP_Report_Feed_Status(uint8_t status);

#endif /* BSP_WIFI_WIFI_H_ */
