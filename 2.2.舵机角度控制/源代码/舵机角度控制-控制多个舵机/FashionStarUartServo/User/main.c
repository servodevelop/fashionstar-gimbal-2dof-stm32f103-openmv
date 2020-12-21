/********************************************************
 * 同时控制两个舵机的角度
 ********************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"

// 使用串口1作为舵机控制的端口
// <接线说明>
// STM32F103 PA9(Tx) 	<----> 串口舵机转接板 Rx
// STM32F103 PA10(Rx) <----> 串口舵机转接板 Tx
// STM32F103 GND 		<----> 串口舵机转接板 GND
// STM32F103 V5 		<----> 串口舵机转接板 5V
// <注意事项>
// 使用前确保已设置usart.h里面的USART1_ENABLE为1
// 设置完成之后, 将下行取消注释
Usart_DataTypeDef* servoUsart = &usart1; 

//// 舵机控制相关的参数
// 时间间隔ms  
// 可以尝试修改设置更小的时间间隔，例如500ms
uint16_t interval = 2000; 
// 舵机执行功率 mV 默认为0	
uint16_t power = 0;
// 设置舵机角度的时候, 是否为阻塞式 
// 0:不等待 1:等待舵机旋转到特定的位置; 
uint8_t wait = 0; 


int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	// 串口初始化
	Usart_Init();
	
	while (1)
    {	
		// 控制舵机云台角度
		FSUS_SetServoAngle(servoUsart, 0, 135.0, interval, power, wait);
		FSUS_SetServoAngle(servoUsart, 1, 45.0, interval, power, wait);
		// 等待动作完成
		SysTick_DelayMs(interval);
		
		// 等待2s
		SysTick_DelayMs(2000);
		
		// 控制舵机旋转到另外一个角度
		FSUS_SetServoAngle(servoUsart, 0, -135.0, interval, power, wait);
		FSUS_SetServoAngle(servoUsart, 1, -45.0, interval, power, wait);
		// 等待动作完成
		SysTick_DelayMs(interval);
		
		// 等待2s
		SysTick_DelayMs(2000);
    }
}

