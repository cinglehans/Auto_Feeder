/*
 * WIFI.c
 *
 *  Created on: Jul 8, 2026
 *      Author: Chanh
 */
#include "WIFI.h"
#include <string.h>  // 用于标准字符串操作
#include <stdio.h>   // 用于 snprintf 拼接和 sscanf 拆解

volatile uint32_t wifi_last_rx_tick = 0; // 上次接收数据的时间戳

extern uint8_t Feed_Flag; // 喂食标志位，由网络指令触发
extern uint8_t Feed_Times; // 喂食次数，由网络指令触发
extern uint8_t wifi_rx_buf[512];         // 接收缓冲区
extern uint16_t wifi_rx_cnt;                  // 缓冲区计数器

/* ==================================================================== */
/*                       应答标志位                             */
/* ==================================================================== */
volatile uint8_t flag_ok = 0;
volatile uint8_t flag_error = 0;
volatile uint8_t flag_connect = 0;
volatile uint8_t flag_closed = 0;
volatile uint8_t flag_send_ok = 0;
volatile uint8_t flag_send_fail = 0;
volatile uint8_t flag_prompt = 0;        // 收到 '>' 提示符
volatile uint8_t flag_wifi_got_ip = 0;     // WiFi连接成功
volatile uint8_t flag_tcp_disconnected = 0; // TCP断开（巴法云专用）
volatile uint8_t flag_link_invalid = 0;      // 收到 "link is not valid"
volatile uint8_t flag_already_connected = 0; // 收到 "ALREADY CONNECTED"
volatile uint8_t flag_busy = 0;				//收到“busy”
volatile uint8_t flag_ipd = 0;           // 收到 +IPD 数据
char ipd_payload[128] = {0};             // 存放解析出的IPD负载
uint16_t ipd_len = 0;

/* ==================================================================== */
/*                       内部私有状态机枚举                             */
/* ==================================================================== */
typedef enum
{
	STEP_HW_RESET_LOW = 0,     // 拉低 RST
	STEP_HW_RESET_WAIT_LOW,    // 等待复位脉冲时间
	STEP_HW_RESET_HIGH,        // 拉高 RST
	STEP_HW_RESET_WAIT_READY,   // 等待模块启动
	STEP_TEST_AT,       // 1. 测试 AT 握手
	STEP_WIFI_MODE,         // 2. 设置为 Station 模式
	STEP_CONNECT_WIFI,      // 3. 连接路由器 WiFi
	STEP_WIFI_STABILIZE,    // 4. 等待 WiFi 稳定
	STEP_NTP_CONFIG,        // 5. 配置 NTP 服务器
	STEP_NTP_GET,           // 6. 抓取并解析外网时间
	STEP_TCP_CONNECT,       // 7. 配置巴法云私钥
	STEP_TCP_ACTIVATE,      // 8. 连接巴法云服务器
	STEP_WAIT_ACTIVATE,		// 9.等待巴法云激活
	STEP_TCP_ONLINE_LOOP   // 11. 稳态长驻在线：长周期监听手机
} ESP_Internal_Step_t;

/* ==================================================================== */
/*                       内部私有变量（加 static 限制作用域）             */
/* ==================================================================== */

static UART_HandleTypeDef *p_esp_uart = NULL; // 绑定的串口指针
static ESP_Internal_Step_t current_step = STEP_TEST_AT; // 状态机当前状态寄存器

static uint8_t is_cmd_sent = 0;                     // 标记当前步骤的指令是否已经发送过
static uint32_t start_tick = 0;                     // 非阻塞计时器：记录每步触发时的毫秒时间戳
static uint8_t AT_OverTimes = 0;

static volatile uint8_t esp_need_reset = 0;   // 外部可设置，触发复位
static ESP_Internal_Step_t resume_step = STEP_TEST_AT; // 复位完成后从哪一步开始
// 配置信息指针
static const char *p_wifi_ssid = NULL;
static const char *p_wifi_pwd = NULL;
static const char *p_cloud_uid = NULL;
static const char *p_cloud_topic = NULL;
static const char *p_User = "your_mqtt_username"; // 替换为实际的 TCP 用户名
static const char *p_Psw = "your_mqtt_password"; // 替换为实际的 TCP 密码

/* ==================================================================== */
/*                       内部私有静态辅助函数                           */
/* ==================================================================== */
/*
 @brief  清空接收缓存，准备接收新数据
 */
static void Clear_Rx_Buffer(void)
{
	__disable_irq(); // 禁用中断，防止接收中断干扰
	memset(wifi_rx_buf, 0, 512);
	wifi_rx_cnt = 0;
	__enable_irq(); // 重新启用中断
}

/**
  * @brief  解析接收缓冲区，提取标志并清空缓冲（在主循环中调用）
  */

/*
 @brief  状态机安全跳转函数，并清除接收缓存
 @param  next_step: 目标状态机步骤
 */
// 状态安全跳转函数
static void Goto_Step(ESP_Internal_Step_t next_step)
{
	current_step = next_step;
	is_cmd_sent = 0; // 解锁标志位，允许新步骤发送命令
}

/*
 @brief  发送 AT 指令到 WIFI模块
 */
static void Send_AT_Cmd(const char *cmd)
{
	HAL_UART_Transmit(p_esp_uart, (uint8_t*) cmd, strlen(cmd), HAL_MAX_DELAY);
	HAL_UART_Transmit(p_esp_uart, (uint8_t*) "\r\n", 2, HAL_MAX_DELAY); // 自动加换行
}

// 检查缓存中是否存在期望的响应字段
static char* Find_Response(const char *target)
{
	if (wifi_rx_cnt == 0)
		return NULL;
	wifi_rx_buf[wifi_rx_cnt >= 512 ? 511 : wifi_rx_cnt] = '\0'; // 强制截断防止越界
	return strstr((char*) wifi_rx_buf, target);
}

// NTP 月份英文转数字
static uint8_t Convert_Month(const char *mstr)
{
	const char *months[] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
			"Nov", "Dec" };
	for (uint8_t i = 0; i < 12; i++)
	{
		if (strcmp(mstr, months[i]) == 0)
			return (i + 1);
	}
	return 1;
}

/*
@brief 解析获取到的IPD语句，获取命令
*/
static void Recv_Parse()
{
	char *p_cmd = strstr(ipd_payload,"cmd=");
		if (p_cmd != NULL)
		{
			uint8_t cmd_id = 0;
			if (sscanf(p_cmd, "cmd=%hhu", &cmd_id) == 1)
			{	
				if (cmd_id == 0)
				{	
					printf("收到心跳信号回复\r\n");
				}
				else if (cmd_id == 1)		// cmd=1 是服务器响应（如激活确认），已在上层处理，此处不做动作
				{
					printf("收到服务器响应 cmd=1: %s\r\n", ipd_payload);
				} 
				else if (cmd_id == 2)		// cmd=2 是控制指令，继续解析 msg
				{
					char *p_msg = strstr(ipd_payload, "msg=");
					if (p_msg != NULL)
					{
						char msg_content[32] = {0};
						uint8_t cmd1 = 0;
						if (sscanf(p_msg, "msg=%hhu", &cmd1) == 1)
						{
							printf("收到控制指令: %s\r\n", msg_content);
							if (cmd1 == 1)
							{
								printf("成功匹配控制指令：开始喂食！\r\n");
								Feed_Flag = 1;
								Feed_Times = 3;
							}
						}
					}
				}
				else
				{
					printf("未知 cmd=%d，忽略\r\n", cmd_id);
				}
			}
			else
			{
				printf("cmd 字段解析失败: %s\r\n", ipd_payload);
			}
		}
		else
		{		
			printf("未找到 cmd 字段，忽略: %s\r\n", ipd_payload);		// 没有 cmd 字段，可能是其他格式，忽略
		}
}
/* ==================================================================== */
/*                       对外公开 API 函数实现                           */
/* ==================================================================== */

/*
@brief  解析接收缓冲区，提取标志并清空缓冲（在主循环中调用）
*/
void WIFI_Rx_Parse(void)
{
    // 只有缓冲区有数据且超过50ms无新数据，才认为一包结束
    if (wifi_rx_cnt > 0 && (HAL_GetTick() - wifi_last_rx_tick) > 100)
    {
        // 拷贝一份到局部数组，避免中断干扰
        uint8_t local_buf[512];
        uint16_t len = wifi_rx_cnt;
        memcpy(local_buf, wifi_rx_buf, len);
        local_buf[len] = '\0';

        // 清空全局缓冲区（关中断保护）
     	Clear_Rx_Buffer();
        // ---- 开始解析 ----
        char *p = (char*)local_buf;

        // 1. 检测 +IPD
        char *p_ipd = strstr(p, "+IPD,");
        if (p_ipd)
		{
            int dlen;
            char data[128] = {0};
            if (sscanf(p_ipd, "+IPD,%d:%127[^\r\n]", &dlen, data) == 2)
			{
				uint16_t actual_len = strlen(data);
				if ((actual_len + 2) >= dlen)
				{
					ipd_len = dlen;
					strncpy(ipd_payload, data, dlen);
					ipd_payload[dlen] = '\0';
					flag_ipd = 1;
				}
				else
				{
					printf("IPD数据不完整（声明%d,实际%d）\r\n",dlen,actual_len);
				}
			}
			else
			{
				printf("IPD格式异常：%s\r\n",p_ipd);
			}
                
        }

        // 2. 检测 CONNECT / CLOSED / OK / ERROR / SEND OK / SEND FAIL
        if (strstr(p, "CONNECT")) flag_connect = 1;
        if (strstr(p, "CLOSED"))  flag_closed = 1;
        if (strstr(p, "OK"))      flag_ok = 1;
        if (strstr(p, "ERROR"))   flag_error = 1;
        if (strstr(p, "SEND OK")) flag_send_ok = 1;
        if (strstr(p, "SEND FAIL")) flag_send_fail = 1;
        if (strstr(p, ">"))       flag_prompt = 1;
		if (strstr(p, "WIFI GOT IP")) flag_wifi_got_ip = 1;
        if (strstr(p, "+MQTTDISCONNECTED")) flag_tcp_disconnected = 1;
		if (strstr(p, "link is not valid")) flag_link_invalid = 1;
		if (strstr(p, "ALREADY CONNECTED")) flag_already_connected = 1;
		if (strstr(p, "busy")) 		flag_busy = 1;

		// printf("WIFI_Rx_Parse: %s\r\n", local_buf); // 调试输出接收到的内容
        // 可以添加其他需要的响应，比如 "ready", "WIFI GOT IP" 等，
        // 但为了减少改动，你可以保留原有的 Find_Response 用于这些特殊字符串。
        // 下面我们会在状态机里保留少量 Find_Response（仅用于非冲突场景），但核心响应改用标志。
    }
}


/*
 @brief  初始化：绑定串口
 @param  huart: 连接 WIFI的串口句柄指针（如 &huart2）
 @param  ssid: Wi-Fi 名称字符串
 @param  pwd:  Wi-Fi 密码字符串
 @param  uid:  云服务器 UID
 @param  topic: 云服务器控制主题
 */
void ESP_Network_Init(UART_HandleTypeDef *huart, const char *ssid,
		const char *pwd, const char *uid, const char *topic)
{
	p_esp_uart = huart;
	p_wifi_ssid = ssid;
	p_wifi_pwd = pwd;
	p_cloud_uid = uid;
	p_cloud_topic = topic;
	p_User = "User"; // 替换为实际的 TCP 用户名
	p_Psw = "psd"; // 替换为实际的 TCP 密码
	HAL_UART_Receive_IT(p_esp_uart, &wifi_rx_buf[wifi_rx_cnt], 1); // 启动串口中断接收
	Goto_Step(STEP_TEST_AT);
}

/*
 @brief  重置状态机
 */
void ESP_Reset_StateMachine(void)
{
	Goto_Step(STEP_TEST_AT);
}

/*
 * @brief  触发一次非阻塞硬件复位
 * @param  after_reset_step: 复位完成后跳转到的状态（通常为 STEP_TEST_AT）
 */
void ESP_Trigger_HardReset(ESP_Internal_Step_t after_reset_step)
{
	esp_need_reset = 1;
	resume_step = after_reset_step;
}

/*
 * @brief  上报喂料状态
 * @param  status: 喂料状态
 * @return void
 */

/*void ESP_Report_Feed_Status(uint8_t status)
 {
 char pub_cmd[128] = {0};
 if (p_cloud_topic == NULL) return;

 if (status == 1) {
 sprintf(pub_cmd, "AT+TCPPUB=0,\"%s\",\"success\",1,0", p_cloud_topic);
 } else {
 sprintf(pub_cmd, "AT+TCPPUB=0,\"%s\",\"error_jam\",1,0", p_cloud_topic);
 }
 Send_AT_Cmd(pub_cmd);
 }
 */
/*

 * @brief  非阻塞核心状态机，处理各状态下的模块命令
 * @param  out_time: 接收最终时间的结构体指针
 * @return ESP_Status_t 返回当前网络状态（返回 ESP_BUSY 表示还在走流程，返回 ESP_OK 表示对时完成）
 */
ESP_Status_t ESP_Network_Task(ESP_Time_t *net_time)
{
	char cmd_buf[160] = { 0 };

	// ========== 硬件复位拦截 ==========
	if (esp_need_reset)
	{
		// 强制将主状态机切换到复位子状态
		current_step = STEP_HW_RESET_LOW;
		esp_need_reset = 0;               // 清除请求标志
		is_cmd_sent = 0;                  // 复位状态使用自己的标记逻辑
	}

	if (esp_need_reset)
	{
		current_step = STEP_HW_RESET_LOW;
		esp_need_reset = 0;               // 清除请求标志
		is_cmd_sent = 0;                  // 复位状态使用自己的标记逻辑
	}

	/*
	 @brief  非阻塞硬件复位状态机
	 @note   复位状态机优先级最高，任何时候都可以触发复位，复位完成后返回到上次的状态机步骤
	 */
	if (current_step <= STEP_HW_RESET_WAIT_READY)
	{
		switch (current_step)
		{
			case STEP_HW_RESET_LOW:
				if (!is_cmd_sent)
				{
					HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin,
							GPIO_PIN_RESET); // 拉低
					start_tick = HAL_GetTick();
					is_cmd_sent = 1;
				}
				else
				{
					if (HAL_GetTick() - start_tick >= 10)
					{  // 至少保持低电平10ms，实际100ms更可靠
						Goto_Step(STEP_HW_RESET_WAIT_LOW);
					}
				}
				return ESP_BUSY;

			case STEP_HW_RESET_WAIT_LOW:
				if (!is_cmd_sent)
				{
					start_tick = HAL_GetTick();
					is_cmd_sent = 1;
				}
				else
				{
					if (HAL_GetTick() - start_tick >= 100)
					{  // 维持100ms低电平复位
						Goto_Step(STEP_HW_RESET_HIGH);
					}
				}
				return ESP_BUSY;

			case STEP_HW_RESET_HIGH:
				if (!is_cmd_sent)
				{
					HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin,
							GPIO_PIN_SET); // 拉高
					start_tick = HAL_GetTick();
					is_cmd_sent = 1;
				}
				else
				{
					if (HAL_GetTick() - start_tick >= 10)
					{  // 确保高电平稳定
						Goto_Step(STEP_HW_RESET_WAIT_READY);
					}
				}
				return ESP_BUSY;

			case STEP_HW_RESET_WAIT_READY:
				if (!is_cmd_sent)
				{
					start_tick = HAL_GetTick();
					is_cmd_sent = 1;
				}
				else
				{
					if (flag_ok)		// 等待模块输出 "ready" 或超时（如5秒）
					{
						// printf("WIFI硬件复位成功，重新开始网络初始化\r\n");
						flag_ok = 0;
						Goto_Step(resume_step);  // 返回正常状态机
						return ESP_BUSY;
					}
					else if (HAL_GetTick() - start_tick > 5000)
					{
						// printf("硬件复位后等待 ready 超时，请检查模块！\r\n");
						Goto_Step(STEP_TEST_AT);		// 超时仍回到测试状态，让正常超时机制处理
						return ESP_ERR_NO_MODULE;
					}
				}
				return ESP_BUSY;

			default:
				break;
		}
	}

	// 运行期间实时检测硬件掉线特征字
	if (Find_Response("CLOSED"))
	{
		Goto_Step(STEP_TCP_CONNECT);
		return ESP_BUSY;
	}

	//WIFI模块连接云服务器状态机
	switch (current_step)
	{

		case STEP_TEST_AT:
			if (is_cmd_sent == 0)
			{
				flag_ok = 0;
				flag_error = 0;
				Send_AT_Cmd("AT"); 
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				// 检测到 OK 响应，进入下一步
				if (flag_ok)
				{
					// printf("WIFI模块 AT 测试成功\r\n");
					Send_AT_Cmd("ATE0"); // 关闭回显，减少干扰
					Goto_Step(STEP_WIFI_MODE);

				}
				else if (flag_error || (HAL_GetTick() - start_tick > 1500))
				{
					// printf("WIFI模块 AT 测试超时\r\n");
					is_cmd_sent = 0;
					AT_OverTimes++;
					if (AT_OverTimes >= 2)
					{
						printf("WIFI模块连续失败，硬件复位\r\n");
						ESP_Trigger_HardReset(STEP_TEST_AT);
						AT_OverTimes = 0;
					}
				}
			}
			return ESP_BUSY;

			// 设置为模块模式为 Station 模式
		case STEP_WIFI_MODE:
			if (is_cmd_sent == 0)
			{
				flag_ok = 0;
				flag_error = 0;
				Send_AT_Cmd("AT+CWMODE=1"); // 发送设置为 Station 模式的命令
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				if (flag_ok)
				{
					// printf("WIFI模块已设置为 Station 模式\r\n");
					Goto_Step(STEP_CONNECT_WIFI);
				}
				else if (flag_error || (HAL_GetTick() - start_tick > 1500))
				{
					printf("WIFI模块设置 Station 模式超时\r\n");
					Goto_Step(STEP_TEST_AT);
					return ESP_ERR_TIMEOUT;
				}
			}
			return ESP_BUSY;

			// 传输WiFi账号密码，选择连接路由器
		case STEP_CONNECT_WIFI:
			if (!is_cmd_sent)
			{
				flag_ok = 0;
				flag_error = 0;
				sprintf(cmd_buf, "AT+CWJAP=\"%s\",\"%s\"", p_wifi_ssid,
						p_wifi_pwd); //
				Send_AT_Cmd(cmd_buf);
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				//
				if (flag_ok)
				{
					printf("WIFI模块已连接 Wi-Fi:%s\r\n", p_wifi_ssid);
					start_tick = HAL_GetTick();
					Goto_Step(STEP_WIFI_STABILIZE);
				}
				else if (flag_error
						|| (HAL_GetTick() - start_tick > 10000))
				{
					printf("WIFI模块连接 Wi-Fi:%s 失败\r\n", p_wifi_ssid);
					is_cmd_sent = 0;
					return ESP_ERR_WIFI;
				}
			}
			return ESP_BUSY;

		case STEP_WIFI_STABILIZE:
			if (is_cmd_sent == 0)
			{
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				if (HAL_GetTick() - start_tick >= 2000)
				{
					Goto_Step(STEP_TCP_CONNECT);
				}
			}
			return ESP_BUSY;

			// 配置 NTP 服务器地址
		case STEP_NTP_CONFIG:
			if (is_cmd_sent == 0)
			{
				Send_AT_Cmd("AT+CIPSNTPCFG=1,8,\"182.254.116.116\"");
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				if (Find_Response("OK"))
				{
					printf("WIFI模块已配置 NTP 服务器\r\n");
					Goto_Step(STEP_NTP_GET);
				}
				else if (HAL_GetTick() - start_tick > 3000)
				{
					printf("WIFI模块配置 NTP 服务器超时\r\n");
					// Goto_Step(STEP_TEST_AT);
					return ESP_ERR_TIMEOUT;
				}
			}
			return ESP_BUSY;

			// 获取网络时间，解析为时间结构体
		case STEP_NTP_GET:
			if (is_cmd_sent == 0)
			{
				Send_AT_Cmd("AT+CIPSNTPTIME?");
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			else
			{
				char *p_ntp = Find_Response("+CIPSNTPTIME:");
				if (p_ntp != NULL)
				{
					printf("ESP8266已获取时间\r\n");
					printf("Raw NTP String: %s\r\n", p_ntp);
					if (strstr(p_ntp, "1970") != NULL)
					{
						printf("NTP 时间无效，可能未同步成功\r\n");
						is_cmd_sent = 0;
						Clear_Rx_Buffer();
						return ESP_ERR_PARSE;
					}
					char week[4] =
					{ 0 };
					char month_str[4] =
					{ 0 };
					int d = 0, h = 0, m = 0, s = 0, y = 0;

					int match = sscanf(p_ntp,
							"+CIPSNTPTIME:%3s %3s %d %d:%d:%d %d", week,
							month_str, &d, &h, &m, &s, &y);
					if (match == 7)
					{
						net_time->year = (uint16_t) y;
						net_time->month = Convert_Month(month_str);
						net_time->day = (uint8_t) d;
						net_time->hour = (uint8_t) h;
						net_time->minute = (uint8_t) m;
						net_time->second = (uint8_t) s;
						printf("解析时间: %04d-%02d-%02d %02d:%02d:%02d\r\n",
								net_time->year, net_time->month, net_time->day,
								net_time->hour, net_time->minute,
								net_time->second);
						Goto_Step(STEP_TCP_CONNECT);
					}
					else
					{
						is_cmd_sent = 0;
						return ESP_ERR_PARSE;
					}
				}
				else if (HAL_GetTick() - start_tick > 4000)
				{
					is_cmd_sent = 0;
					return ESP_ERR_TIMEOUT;
				}
			}
			return ESP_BUSY;

			// 配置 TCP 连接
		case STEP_TCP_CONNECT:
			if (is_cmd_sent == 0)
			{
				flag_connect = 0;
				flag_error = 0;
				flag_closed = 0;
				flag_already_connected = 0;
				flag_busy = 0;

				sprintf(cmd_buf,"AT+CIPSTART=\"TCP\",\"tcp.bemfa.com\",8344");		//使用TCP连接命令
				Send_AT_Cmd(cmd_buf);

				start_tick = HAL_GetTick();     //记录总生存时间的起点
				is_cmd_sent = 1;
				return ESP_BUSY;
			}
			else
			{	
				if (flag_busy)
				{
					printf("模块忙，等待 2 秒后重试\r\n");
					flag_busy = 0;
					is_cmd_sent = 0;
					start_tick = HAL_GetTick() + 1000; // 让下一次进入时延时生效
					return ESP_BUSY;
				}
				if (flag_connect || flag_already_connected)
				{
					// printf("TCP握手成功！\r\n");
					Goto_Step(STEP_TCP_ACTIVATE);
					return ESP_BUSY;
				}
				if (flag_error || flag_closed || (HAL_GetTick() - start_tick > 10000))
				{
					printf("TCP 握手失败，重试:\r\n");
					Send_AT_Cmd("AT+CIPCLOSE");
					is_cmd_sent = 0;
					start_tick = HAL_GetTick(); // 重置超时
				}
				// 注意：原代码中处理 busy 的逻辑可以保留，但需要根据实际情况调整，这里简化为超时重试
			}
			return ESP_BUSY;

		// 向巴法云服务器发送 UID 和 Topic 进行登录激活
		case STEP_TCP_ACTIVATE:
		
			static uint8_t sub_step = 0;      // 0:发CIPSEND, 1:等待>, 2:发数据, 3:等待SEND OK
			static char subscribe_data[128];
			static uint16_t data_len = 0;
			if (!is_cmd_sent)
			{
				// 构造订阅数据
				snprintf(subscribe_data, sizeof(subscribe_data), "cmd=1&uid=%s&topic=%s\r\n", p_cloud_uid, p_cloud_topic);
				data_len = strlen(subscribe_data);
				sub_step = 0;
				is_cmd_sent = 1;
				
				flag_prompt = 0;		// 清空相关标志
				flag_send_ok = 0;
				flag_send_fail = 0;
				flag_error = 0;
				flag_link_invalid = 0;
				// 不清空缓冲区
			}
			switch(sub_step)
			{
				case 0:		//发送CIPSEND命令
					sprintf(cmd_buf, "AT+CIPSEND=%d", data_len);
					Send_AT_Cmd(cmd_buf);
					start_tick = HAL_GetTick();
					sub_step = 1;
					break;
				case 1:		//等待'>'提示符
					 // 检测连接错误
					if (flag_link_invalid || flag_error || flag_closed)
					{
						printf("连接无效，重新建立TCP连接\r\n");
						// printf("错误代码为：%s\r\n",local_buf);
						flag_link_invalid = 0; 
						flag_error = 0; 
						flag_closed = 0;
						sub_step = 0; 
						is_cmd_sent = 0;
						Goto_Step(STEP_TCP_CONNECT);		//直接跳回重连
						return ESP_BUSY;
					}

					if (flag_prompt)
					{
						flag_prompt = 0;
						HAL_UART_Transmit(p_esp_uart, (uint8_t*)subscribe_data, data_len, HAL_MAX_DELAY);// 发送数据（不要加额外换行，因为subscribe_data已有\r\n）
						start_tick = HAL_GetTick();
						sub_step = 2;
					}
					else if (HAL_GetTick() - start_tick > 2000)
					{
						printf("等待 '>' 超时，连接失效，重新连接\r\n");
						Send_AT_Cmd("AT+CIPCLOSE");		//关闭连接
						sub_step = 0;  // 重试
						is_cmd_sent = 0; // 允许重新开始
						Goto_Step(STEP_TCP_CONNECT);
						return ESP_BUSY;
					}
					break;
				case 2:		//等待SEND OK或失败
					// 同样检测连接错误
					if (flag_link_invalid || flag_error || flag_closed)
					{
						printf("发送数据时连接断开，重连\r\n");
						flag_link_invalid = 0; flag_error = 0; flag_closed = 0;
						sub_step = 0; is_cmd_sent = 0;
						Goto_Step(STEP_TCP_CONNECT);
						return ESP_BUSY;
					}
					if (flag_send_ok)
					{
						flag_send_ok = 0;
						// printf("订阅数据发送成功，等待服务器激活\r\n");
						Goto_Step(STEP_WAIT_ACTIVATE);
					}
					else if (flag_send_fail || flag_error || (HAL_GetTick() - start_tick > 3000))
					{
						printf("发送失败，重试\r\n");
						sub_step = 0;
						is_cmd_sent = 0;
					}
					break;
			}
			return ESP_BUSY;
		
			
		// 等待巴法云激活响应
		case STEP_WAIT_ACTIVATE:
			if (is_cmd_sent == 0)
			{
				start_tick = HAL_GetTick();
				is_cmd_sent = 1;
			}
			
			if (flag_ipd)
			{
				flag_ipd = 0; // 清除标志，准备下一次接收
				if (strstr(ipd_payload, "cmd=1&res=1") != NULL)
				{
					printf("已成功激活上线！\r\n");
					Goto_Step(STEP_TCP_ONLINE_LOOP);
					return ESP_BUSY;
				}
				else
				{
					// 如果不是激活响应，忽略并继续等待（不清除flag_ipd? 已清）
					// 但可能还有其他IPD，这里不做处理，等待下一次解析
				}
			}
			else if (HAL_GetTick() - start_tick > 3000)
			{
				printf("激活响应超时！重连\r\n");
				Goto_Step(STEP_TCP_CONNECT);
				return ESP_BUSY;
			}
			return ESP_BUSY;

			// 稳态在线，持续获取信息，是否有新传入指令，并向应用层输出 OK
		case STEP_TCP_ONLINE_LOOP:

			// 检查连接断开
			if (flag_closed || flag_error || flag_tcp_disconnected)
			{
				printf("TCP 连接断开，重新握手\r\n");
				flag_closed = 0; 
				flag_error = 0;
				flag_tcp_disconnected = 0;
				Goto_Step(STEP_TCP_CONNECT);
				return ESP_BUSY;
			}

			// 处理IPD数据
			if (flag_ipd)
			{
				flag_ipd = 0;
				printf("原始下发数据: %s\r\n", ipd_payload);

				Recv_Parse();
			}

			// 心跳：每30秒发送一次ping（使用子状态简化版）
			static uint32_t last_heartbeat_tick = 0;
			if (HAL_GetTick() - last_heartbeat_tick > 59000)
			{
				Send_AT_Cmd("AT+CIPSEND=4");		// 发送 AT+CIPSEND=4
				HAL_Delay(20);		// 简单等待20ms让模块准备
				HAL_UART_Transmit(p_esp_uart, (uint8_t*)"ping\r\n", 6, HAL_MAX_DELAY);		// 发送 "ping\r\n"
				last_heartbeat_tick = HAL_GetTick();
				// 注意：这里不会等待SEND OK，因为巴法云允许，如果希望可靠，可参考上面的子状态方式
			}

			// 60s无数据重连（保留原逻辑）
			if (HAL_GetTick() - wifi_last_rx_tick > 60000)
			{
				printf("长时间无数据，主动重连\r\n");
				Send_AT_Cmd("AT+CIPCLOSE");
				Goto_Step(STEP_TCP_CONNECT);
				return ESP_BUSY;
			}

			return ESP_OK;
		

		default:
			return ESP_BUSY;

	}
}

