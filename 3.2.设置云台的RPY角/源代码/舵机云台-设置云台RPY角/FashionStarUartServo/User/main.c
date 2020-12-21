/***************************************************
* 舵机云台控制-设置舵机云台关节角度
 *   设置舵机为阻尼模式后, 用手旋转云台的两个舵机,
 *   串口2每隔一段时间打印一下舵机角度信息
 ***************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"
#include "gimbal.h"


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

float servoSpeed = 200.0; // 云台旋转速度 (单位: °/s)
int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	Usart_Init(); // 串口初始化
	// 云台初始化
	Gimbal_Init(servoUsart);
	// 等待2s
	SysTick_DelayMs(2000);	
	
	while (1){
		// 设置云台目标位姿
		Gimbal_SetYaw(servoUsart, 60, servoSpeed);
		Gimbal_SetPitch(servoUsart, 45, servoSpeed);
		// 等待云台旋转到目标位置
		Gimbal_Wait(servoUsart);
		
		// 延时1s
		SysTick_DelayMs(1000);
		
		// 设置云台目标位姿
		Gimbal_SetYaw(servoUsart, -60, servoSpeed);
		Gimbal_SetPitch(servoUsart, -45, servoSpeed);
		// 等待云台旋转到目标位置
		Gimbal_Wait(servoUsart);
		
		// 延时1s
		SysTick_DelayMs(1000);
    }
}
