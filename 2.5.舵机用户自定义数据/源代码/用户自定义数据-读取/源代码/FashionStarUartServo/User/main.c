/********************************************************
 * 读取用户自定义数据 - 读取舵机角度上限
 * 	
 * <输出日志>
 *	=====read user data======
 *	status code: 0
 *	read sucess, angle limit high: 135.000000
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
float angleLimitHigh = 0; 	// 舵机角度上限

int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	// 串口初始化
	Usart_Init();
	
	// 读取用户自定义数据
	// 数据表里面的数据字节长度一般为1个字节/2个字节/4个字节
	// 查阅通信协议可知,舵机角度上限的数据类型是有符号短整型(UShort, 对应STM32里面的int16_t),长度为2个字节
	// 所以这里设置value的数据类型为int16_t
	int16_t value; 
	uint8_t dataSize; 
	// 传参数的时候, 要将value的指针强行转换为uint8_t
	statusCode = FSUS_ReadData(servoUsart, servoId, FSUS_PARAM_ANGLE_LIMIT_HIGH, (uint8_t *)&value, &dataSize);
	printf("=====read user data======\r\n status code: %d\r\n", statusCode);
	if (statusCode == FSUS_STATUS_SUCCESS){
		// 用户自定义表里面，角度的单位是0.1度. 需要改写为度
		angleLimitHigh = value / 10.0;
		printf("read sucess, angle limit high: %f\r\n", angleLimitHigh);
	}else{
		printf("fail\r\n");
	}
	
	// 死循环
	while (1);
}
