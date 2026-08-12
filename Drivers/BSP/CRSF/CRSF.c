/*
 * CRSF.c
 *
 *  Created on: Apr 17, 2026
 *      Author: Chanh
 */
#include "CRSF.h"
#include "string.h"

/* 内部函数声明 */
static crsf_status_t crsf_process_frame(crsf_parser_t *parser, uint8_t *buf, uint8_t len);
static uint16_t crsf_extract_channel(const uint8_t *payload, uint8_t ch_idx);

/* 初始化解析器 */
void crsf_parser_init(crsf_parser_t *parser)
{
    if (!parser) return;
    memset(parser, 0, sizeof(crsf_parser_t));
    parser->expected_len = 0;
    parser->frame_ready = 0;
}

/* 逐字节解析 */
crsf_status_t crsf_parse_byte(crsf_parser_t *parser, uint8_t data)
{
    if (!parser) return CRSF_NO_SYNC;

    /* 等待同步字节 */
    if (parser->rx_index == 0 && data != CRSF_SYNC_Byte)
    {
        return CRSF_NO_SYNC;
    }

    /* 存储字节 */
    parser->rx_buffer[parser->rx_index++] = data;

    /* 第二个字节包含长度信息 */
    if (parser->rx_index == 2)
    {
        parser->expected_len = parser->rx_buffer[1] + 2;  // +2 for sync and length bytes
        if (parser->expected_len > sizeof(parser->rx_buffer))
        {
            /* 长度非法，重置状态机 */
            parser->rx_index = 0;
            parser->expected_len = 0;
            return CRSF_INVALID_LENGTH;
        }
    }

    /* 检查帧是否完整 */
    if (parser->rx_index > 1 && parser->rx_index == parser->expected_len)
    {
        /* 完整帧接收完成，尝试处理 */
        crsf_status_t status = crsf_process_frame(parser, parser->rx_buffer, parser->rx_index);
        /* 无论处理成功与否，重置状态机准备接收下一帧 */
        parser->rx_index = 0;
        parser->expected_len = 0;

        if (status == CRSF_OK)
        {
            parser->frame_ready = 1;
        }
        return status;
    }

    return CRSF_FRAME_INCOMPLETE;
}

/* 处理完整帧：校验、提取通道 */
static crsf_status_t crsf_process_frame(crsf_parser_t *parser, uint8_t *buf, uint8_t len)
{
    /* CRC 校验：从 type 字节开始到 payload 结束（不包含 sync 和 length，也不包含最后的 CRC 字节） */
    uint8_t crc_calc = crsf_crc8(&buf[2], len - 3);
    if (crc_calc != buf[len - 1]) {
        return CRSF_CRC_ERROR;
    }

    /* 检查帧类型 */
    if (buf[2] != CRSF_Type_RC) {
        return CRSF_UNSUPPORTED_TYPE;
    }

    /* 提取通道数据，payload 从索引 3 开始，长度为 22 字节 */
    const uint8_t *payload = &buf[3];
    for (uint8_t i = 0; i < CRSF_Channels_Num; i++) {
        parser->channels[i] = crsf_extract_channel(payload, i);
    }

    return CRSF_OK;
}

/* 从 payload 中提取第 ch_idx 个 11 位通道值（小端跨字节） */
static uint16_t crsf_extract_channel(const uint8_t *payload, uint8_t ch_idx)
{
    // 1. 计算起始位
    const uint16_t start_bit = ch_idx * 11;

    // 2. 计算字节索引（低地址）和位偏移
    const uint8_t byte_idx = start_bit / 8;
    const uint8_t bit_offset = start_bit % 8; // 在字节内的起始位置（0-7）

    // 3. 从内存中取出 3 个字节，并正确组合成 32 位整数
    //    关键点：payload[byte_idx] 是低字节，payload[byte_idx+2] 是高字节
    uint32_t raw = 0;
    raw |= (uint32_t)payload[byte_idx];       // 低 8 位
    raw |= (uint32_t)payload[byte_idx + 1] << 8;  // 中 8 位
    raw |= (uint32_t)payload[byte_idx + 2] << 16; // 高 8 位

    // 4. 右移，把有效数据对齐到 Bit 0
    //    我们需要把前导的 bit_offset 个垃圾位丢掉
    raw >>= bit_offset;

    // 5. 截取低 11 位
    return raw & 0x07FF;
}


/* 检查是否有新帧 */
uint8_t crsf_has_new_frame(crsf_parser_t *parser)
{
    if (!parser) return 0;
    uint8_t ready = parser->frame_ready;
    parser->frame_ready = 0;   /* 清除标志 */
    return ready;
}

/* 获取所有通道值 */
void crsf_get_channels(crsf_parser_t *parser, crsf_channels_t out_channels)
{
    if (!parser || !out_channels) return;
    memcpy(out_channels, parser->channels, sizeof(crsf_channels_t));
}

/* 获取单个通道值 */
uint16_t crsf_get_channel(crsf_parser_t *parser, uint8_t ch_idx)
{
    if (!parser || ch_idx >= CRSF_Channels_Num) return 0;
    return parser->channels[ch_idx];
}

/* CRC-8 计算 */
uint8_t crsf_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0xD5;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

/* 辅助映射函数 */
uint16_t crsf_map_raw_to_pulse(uint16_t raw, uint16_t pulse_min, uint16_t pulse_max)
{
    if (raw < CRSF_CHANNEL_MIN) raw = CRSF_CHANNEL_MIN;
    if (raw > CRSF_CHANNEL_MAX) raw = CRSF_CHANNEL_MAX;
    uint32_t tmp = (uint32_t)(raw - CRSF_CHANNEL_MIN) * (pulse_max - pulse_min);
    return (uint16_t)(tmp / (CRSF_CHANNEL_MAX - CRSF_CHANNEL_MIN) + pulse_min);
}

