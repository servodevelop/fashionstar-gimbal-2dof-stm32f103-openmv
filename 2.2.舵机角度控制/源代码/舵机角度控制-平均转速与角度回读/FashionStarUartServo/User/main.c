/********************************************************
 * 控制舵机舵机角度-平均转速与角度回读
 * 1. 查询当前角度, 根据平均转速还有目标角度估计执行周期
 * 2. 通过周期性查询舵机角度判断舵机是否到达目标角度。
 * 3. 测试舵机的死区(Dead Block)/稳态误差
 ********************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"

// 使用串口1作为舵机控制的端口
// <接线说明>
// STM32F103 PA9(Tx) 	<----> 串口舵机转接板 Rx
// STM32F103 PA10(Rx) 	<----> 串口舵机转接板 Tx
// STM32F103 GND 		<----> 串口舵机转接板 GND
// STM32F103 V5 		<----> 串口舵机转接板 5V
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

// 舵机控制相关的参数
uint8_t servoId = 0; 	// 舵机的ID
float curAngle = 0;		// 舵机当前所在的角度
float nextAngle = 0; 	// 舵机的目标角度
uint16_t speed = 200; 	// 舵机的转速 单位 °/s
uint16_t interval = 0; 	// 舵机旋转的周期
uint16_t power = 0; 	// 舵机执行功率 mV 默认为0	W
uint8_t wait = 0;  		// 0:不等待 1:等待舵机旋转到特定的位置;
// 舵机角度死区, 如果舵机当前角度跟
// 目标角度相差小于死区则代表舵机到达目标角度, 舵机不再旋转
// <注意事项>
// 		死区跟舵机的型号有关系, 取决于舵机固件的设置, 不同型号的舵机会有差别
float servoDeadBlock = 1.0; 

// 查询舵机的角度
uint16_t calcIntervalMs(uint8_t servoId, float nextAngle, float speed){
	// 读取一下舵机的角度
	FSUS_QueryServoAngle(servoUsart, servoId, &curAngle);
	// 计算角度误差
	float dAngle =  (nextAngle > curAngle) ? (nextAngle - curAngle) : (curAngle - nextAngle);
	// 计算所需的时间
	return (uint16_t)((dAngle / speed) * 1000.0);
}

// 等待舵机进入空闲状态IDLE, 即舵机到达目标角度
void waitUntilServoIDLE(uint8_t servoId, float nextAngle){
	
	while(1){
		// 读取一下舵机的角度
		FSUS_QueryServoAngle(servoUsart, servoId, &curAngle);
		
		// 判断舵机是否达到目标角度
		float dAngle =  (nextAngle > curAngle) ? (nextAngle - curAngle) : (curAngle - nextAngle);
		
		// 打印一下当前的舵机角度
		printf("curAngle: %f dAngle: %f\r\n", curAngle, dAngle);
		
		// 判断是否小于死区
		if (dAngle <= servoDeadBlock){
			break;
		}
		// 等待一小段时间
		SysTick_DelayMs(5);
	}
}


int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	// 串口初始化
	Usart_Init();
	
	while (1)
    {	
		// 设置舵机的目标角度
		nextAngle = 120.0;
		// 根据转速还有角度误差计算周期
		interval = calcIntervalMs(servoId, nextAngle, speed);
		printf("Set Servo %f-> %f", curAngle, nextAngle);
		// 控制舵机角度
		FSUS_SetServoAngle(servoUsart, servoId, nextAngle, interval, power, wait);
		// SysTick_DelayMs(interval);
		SysTick_DelayMs(5);
		waitUntilServoIDLE(servoId, nextAngle);
		
		// 等待1s 看舵机死区范围
		SysTick_DelayMs(1000);
		// 读取一下舵机的角度
		FSUS_QueryServoAngle(servoUsart, servoId, &curAngle);
		printf("Final Angle: %f", curAngle);
		SysTick_DelayMs(1000);
		
		// 设置舵机的目标角度
		nextAngle = -120;
		// 根据转速还有角度误差计算周期
		interval = calcIntervalMs(servoId, nextAngle, speed);
		// 控制舵机角度
		FSUS_SetServoAngle(servoUsart, servoId, nextAngle, interval, power, wait);
		// 需要延时一会儿，确保舵机接收并开始执行舵机控制指令
		// 如果马上发送舵机角度查询信息,新发送的这条指令可能会覆盖舵机角度控制信息
		SysTick_DelayMs(5);
		waitUntilServoIDLE(servoId, nextAngle);
		
		// 等待1s 看舵机死区范围
		SysTick_DelayMs(1000);
		// 读取一下舵机的角度
		FSUS_QueryServoAngle(servoUsart, servoId, &curAngle);
		printf("Final Angle: %f", curAngle);
		SysTick_DelayMs(1000);
    }
}
