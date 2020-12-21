/***************************************************
* 舵机云台阻尼模式下采集标定所需的数据
 *   设置舵机为阻尼模式后, 用手旋转云台的两个舵机,
 *   串口2每隔一段时间打印一下舵机角度信息
 ***************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"

#define SERVO_DOWN 0 // 云台下方的舵机ID
#define SERVO_UP 1 // 云台上方的舵机ID

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

FSUS_STATUS statusCode; // 请求包的状态码

float servoDownAngle = 0; // 下方舵机的角度
float servoUpAngle = 0; 	// 上方舵机的角度

// 云台初始化-设置为阻尼模式
void InitGimbal(void){
	const uint16_t power = 500; // 阻尼模式下的功率，功率越大阻力越大
	Usart_Init(); // 串口初始化
	FSUS_DampingMode(servoUsart, SERVO_DOWN, power); // 设置舵机0为阻尼模式
	FSUS_DampingMode(servoUsart, SERVO_UP, power); // 设置舵机1为阻尼模式
}

// 更新舵机云台舵机的角度
void UpdateGimbalSrvAngle(void){
	uint8_t code;
	code = FSUS_QueryServoAngle(servoUsart, SERVO_DOWN, &servoDownAngle);
	printf("status code : %d \r\n", code);
	code = FSUS_QueryServoAngle(servoUsart, SERVO_UP, &servoUpAngle);
	printf("status code : %d \r\n", code);
}


int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	InitGimbal();
	
	while (1){	
		// 更新云台舵机角度
		UpdateGimbalSrvAngle();
		// 打印一下当前舵机的角度信息
		printf("Servo Down: %.2f;  Servo Up: %.2f \r\n", servoDownAngle, servoUpAngle);
		// 等待200ms
		SysTick_DelayMs(200);
    }
}
