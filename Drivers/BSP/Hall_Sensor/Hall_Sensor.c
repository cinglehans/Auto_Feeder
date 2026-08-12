/*
 * Hall_Sensor.c
 *
 *  Created on: Jun 22, 2026
 *      Author: Chanh
 */

#include "Hall_Sensor.h"

/*
@brief  对霍尔传感器状态进行读取
@param  Pin Port    GPIO端口
@param  Pin Number  GPIO引脚号
@param  expectedState 触发状态（GPIO_PIN_RESET或GPIO_PIN_SET）
@return 霍尔传感器状态 1:触发 0:未触发
*/
uint8_t Hall_Sensor_Read(GPIO_TypeDef* Port, uint16_t Pin, GPIO_PinState expectedState)
{
    uint8_t cnt = 0;
    // 进行多次读取以增加可靠性
    for(int i = 0; i < 10; i++)
    {
        if(HAL_GPIO_ReadPin(Port, Pin) == expectedState)
        {
            cnt++;
        }
    }
    if(cnt > 5)
    {
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET); // 蜂鸣器发声提示
        HAL_Delay(100); // 延时100ms
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET); // 关闭蜂鸣器
        return 1; // 霍尔传感器被触发
    }
    else
    {
        return 0; // 霍尔传感器未被触发
    }
    // if(Hall_Seal_Read() == expectedState)
    // {
    //     HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET); // 蜂鸣器发声提示
    //     HAL_Delay(100); // 延时100ms
    //     HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET); // 关闭蜂鸣器
    //     return 1; // 霍尔传感器被触发
    // }
    // else
    // {
    //     return 0; // 霍尔传感器未被触发
    // }

}

/*
@brief  密封状态判定，进入密封状态后刹车制动
*/
uint8_t Hall_Seal_Check(void)
{
    return Hall_Sensor_Read(Hall_Seal_Port, Hall_Seal_Pin, Hall_Triggered);
}

/*
@brief  放粮状态判定，进入放粮状态后启动放粮机制：停止3秒
*/
uint8_t Hall_Feed_Check(void)
{
    return Hall_Sensor_Read(Hall_Feed_Port, Hall_Feed_Pin, Hall_Triggered);
}