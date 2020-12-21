/***************************************************
 * 舵机云台控制-按键中断控制云台
 ***************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"
#include "gimbal.h"
#include "button.h"

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

float servoSpeed = 100.0; // 云台旋转速度 (单位: °/s)

int main (void)
{
	SysTick_Init(); 		// 嘀嗒定时器初始化
	Usart_Init(); 			// 串口初始化
	Gimbal_Init(servoUsart);// 云台初始化
	Button_Init(); 			// 按键初始化
	SysTick_DelayMs(2000); 	// 等待2s	
	
	while (1){
		if (__fabs(curYaw - nextYaw) > SERVO_DEAD_BLOCK){
			printf("nextYaw: %.1f  nextPitch: %.1f\r\n", nextYaw, nextPitch);
			Gimbal_SetYaw(servoUsart, nextYaw, servoSpeed);
		}
		if (__fabs(curPitch - nextPitch) > SERVO_DEAD_BLOCK){
			printf("nextYaw: %.1f  nextPitch: %.1f\r\n", nextYaw, nextPitch);
			Gimbal_SetPitch(servoUsart, nextPitch, servoSpeed);
		}
		
		// 延时5ms 需要必要的延时，防止指令被覆盖
		SysTick_DelayMs(5);
    }
}
