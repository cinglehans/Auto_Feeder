/*
 * CRSF.h
 *
 *  Created on: Apr 17, 2026
 *      Author: Chanh
 */

#ifndef BSP_CRSF_CRSF_H_
#define BSP_CRSF_CRSF_H_

#include "stdint.h"


/*通道数量*/
#define CRSF_Channels_Num	16

/* 通道原始值范围（11位） */
#define CRSF_CHANNEL_MIN 172
#define CRSF_CHANNEL_MAX 1811

/* 帧类型定义 */
#define CRSF_SYNC_Byte       0xC8
#define CRSF_Type_RC         0x16

/* 解析结果状态码 */
typedef enum {
    CRSF_OK = 0,
    CRSF_NO_SYNC,
    CRSF_INVALID_LENGTH,
    CRSF_CRC_ERROR,
    CRSF_UNSUPPORTED_TYPE,
    CRSF_FRAME_INCOMPLETE
} crsf_status_t;

/* 通道值数组类型，原始值范围 172~1811 */
typedef uint16_t crsf_channels_t[CRSF_Channels_Num];

/* 解析器上下文（状态机） */
typedef struct {
    uint8_t rx_buffer[64];      // 接收缓冲区
    uint8_t rx_index;           // 当前接收位置
    uint8_t expected_len;       // 预期帧总长度
    uint8_t frame_ready;        // 完整帧就绪标志
    crsf_channels_t channels;   // 最新解析出的通道值
} crsf_parser_t;

/* 初始化解析器 */
void crsf_parser_init(crsf_parser_t *parser);

/* 喂入一个字节，进行状态机解析 */
/* 返回值：CRSF_OK 表示当前没有完整帧；当返回 CRSF_FRAME_INCOMPLETE 时表示帧已就绪（但尚未处理） */
/* 实际上我们返回处理状态，若帧完成则内部自动调用解析，并设置 frame_ready 标志 */
crsf_status_t crsf_parse_byte(crsf_parser_t *parser, uint8_t data);

/* 检查是否有新的有效帧解析完成（获取最新通道数据） */
uint8_t crsf_has_new_frame(crsf_parser_t *parser);

/* 获取最新的通道值（拷贝到用户数组） */
void crsf_get_channels(crsf_parser_t *parser, crsf_channels_t out_channels);

/* 获取指定通道的最新原始值 */
uint16_t crsf_get_channel(crsf_parser_t *parser, uint8_t ch_idx);

/* 计算 CRSF 使用的 CRC-8（多项式 0xD5） */
uint8_t crsf_crc8(const uint8_t *data, uint8_t len);

/* 辅助函数：将原始值映射到 PWM 脉冲宽度（微秒） */
/* 输入: raw 172~1811, out_min 1000, out_max 2000 */
uint16_t crsf_map_raw_to_pulse(uint16_t raw, uint16_t pulse_min, uint16_t pulse_max);


#endif /* BSP_CRSF_CRSF_H_ */
