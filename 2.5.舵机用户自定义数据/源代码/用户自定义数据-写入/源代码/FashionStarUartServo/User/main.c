/********************************************************
 * 写入用户自定义数据 - 写入舵机角度上限与下限，并开启舵机角度限制开关
 * 	
 ********************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"

// 使用串口1作为舵机控制的端口
// <接线说明>
// STM32F103 PA9(Tx)  <----> 串口舵机转接板 Rx
// STM32F103 PA10(Rx) <----> 串口舵机转接板 Tx
// STM32F103 GND 	  <----> 串口舵机转接板 GND
// STM32F103 V5 	  <----> 串口舵机转接板 5V
// <注意事项>
// 使用前确保已设置usart.h里面的USART1_ENABLE为1
// 设置完成之后, 将下行取消注释
Usart_DataTypeDef* servoUsart = &usart1; 

// 使用串口2作为日志输出的端口
// <接线说明>
// STM32F103 PA2(Tx) <----> USB转TTL Rx
// STM32F103 PA3(Rx) <----> USB转TTL Tx
// STM32F103 GND 	 <----> USB转TTL GND
// STM32F103 V5 	 <----> USB转TTL 5V (可选)
// <注意事项>
// 使用前确保已设置usart.h里面的USART2_ENABLE为1
Usart_DataTypeDef* loggingUsart = &usart2;

// 重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	while((loggingUsart->pUSARTx->SR&0X40)==0){}
	/* 发送一个字节数据到串口 */
	USART_SendData(loggingUsart->pUSARTx, (uint8_t) ch);
	/* 等待发送完毕 */
	// while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);		
	return (ch);
}

uint8_t servoId = 0;  		// 连接在转接板上的串口舵机ID号
FSUS_STATUS statusCode;  	// 状态码
// 限位角度
float angleLimitHigh = 90.0; 	// 舵机角度上限 (默认值 135)
float angleLimitLow = -90.0; 	// 舵机角度下限 (默认值-135)
uint8_t angleLimitSwitch = 0x01; // 0x01: 开启限位; 0x00: 关闭限位

uint16_t value;
int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	// 串口初始化
	Usart_Init();
	
	// 写入舵机角度上限
	value = (int16_t)(angleLimitHigh*10); // 舵机角度上限 转换单位为0.1度
	statusCode = FSUS_WriteData(servoUsart, servoId, FSUS_PARAM_ANGLE_LIMIT_HIGH, (uint8_t *)&value, 2);
	printf("write angle limit high = %f, status code: %d\r\n", angleLimitHigh, statusCode);
	
	// 写入舵机角度下限制
	value = (int16_t)(angleLimitLow*10); // 舵机角度下限 转换单位为0.1度
	statusCode = FSUS_WriteData(servoUsart, servoId, FSUS_PARAM_ANGLE_LIMIT_LOW, (uint8_t *)&value, 2);
	printf("write angle limit low = %f, status code: %d\r\n", angleLimitLow, statusCode);
	
	// 打开舵机角度限位开关, 配置生效
	statusCode = FSUS_WriteData(servoUsart, servoId, FSUS_PARAM_ANGLE_LIMIT_LOW, &angleLimitSwitch, 1);
	printf("enable angle limit mode, status code: %d\r\n", statusCode);
	
	while (1){
		// 控制舵机角度
		FSUS_SetServoAngle(servoUsart, servoId, 90.0, 2000, 0, 1);
		FSUS_SetServoAngle(servoUsart, servoId, -90.0, 2000, 0, 1);
		// 写入的舵机角度超出了预设的限位, 查看效果
		// FSUS_SetServoAngle(servoUsart, servoId, 135.0, 2000, 0, 1);
		// FSUS_SetServoAngle(servoUsart, servoId, -135.0, 2000, 0, 1);
	}
}


