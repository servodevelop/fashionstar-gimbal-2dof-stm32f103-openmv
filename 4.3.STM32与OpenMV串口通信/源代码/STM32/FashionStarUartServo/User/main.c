/***************************************************
 * STM32与OpenMV串口通信
 *   OpenMV识别到色块之后, 通过串口通信将色块的中心
 *   坐标发送给STM32, STM32解析串口的字节流数据, 
 *   并将解析得到的色块坐标通过USB转TTL输出到串口调试助手上.
 ***************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"
#include "gimbal.h"
#include "button.h"

#define TRUE 1
#define FALSE 0

#define IMG_WIDTH 320 // blob画面分辨率 宽度
#define IMG_HEIGHT 240 // blob画面分辨率 高度
#define BLOB_PKG_LEN 7 // blob数据包的长度
#define BLOB_PKG_HEADER 0xFF // 帧头
#define BLOB_PKG_CMD_ID 0xF1 // 指令ID

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
// 使用串口3接收来自blob的消息
// <接线说明>
// STM32F103 PB10(Tx) <----> blob P5 (UART3 Rx)
// STM32F103 PB11(Rx) <----> blob P4 (UART3 Tx)
// STM32F103 GND 	  <----> blob GND
// STM32F103 V5 	  <----> blob Vin (5v)
// <注意事项>
// 使用前确保已设置usart.h里面的USART3_ENABLE为1
Usart_DataTypeDef* blobUsart = &usart3;


uint8_t blobPkgBuf[BLOB_PKG_LEN]; // blob数据帧缓冲区
uint8_t blobPkgIdx = 0;

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

uint8_t hasBlob = FALSE; // 画面中有无色块
uint16_t blobCx = 0; // 色块中心的x坐标
uint16_t blobCy = 0; // 色块中心的y坐标

// 更新色块的信息
void updateBlobInfo(Usart_DataTypeDef* blobUsart){
	uint8_t tempByte;
	while(RingBuffer_GetByteUsed(blobUsart->recvBuf)){
		// 弹出队首元素
		tempByte = RingBuffer_Pop(blobUsart->recvBuf);
		
		if (blobPkgIdx == 0 && tempByte != BLOB_PKG_HEADER){
			// 帧头还未接收且帧头不匹配
			continue;
		}else if(blobPkgIdx == 1 && tempByte != BLOB_PKG_CMD_ID){
			// 数据指令不匹配
			blobPkgIdx = 0;
			continue;
		}
		
		// 缓冲区内追加数据
		blobPkgBuf[blobPkgIdx] = tempByte;
		blobPkgIdx += 1;
		
		if (blobPkgIdx >= BLOB_PKG_LEN){
			// 数据接收完成, 解析更新数据
			hasBlob = blobPkgBuf[2];
			blobCx =(uint16_t)(blobPkgBuf[3] | blobPkgBuf[4] << 8);
			blobCy = (uint16_t)(blobPkgBuf[5] | blobPkgBuf[6] << 8);
			
			blobPkgIdx = 0; // 游标清零
		}
	}
}
	
int main (void)
{
	SysTick_Init(); 		// 嘀嗒定时器初始化
	Usart_Init(); 			// 串口初始化
	Gimbal_Init(servoUsart);// 云台初始化
	
	while(1){
		// 更新Buffer
		if (RingBuffer_GetByteUsed(blobUsart->recvBuf) >= BLOB_PKG_LEN) {
			updateBlobInfo(blobUsart);
		}
		// 打印日志
		printf("has blob: %d blob_cx: %d blob_cy: %d\r\n", hasBlob, blobCx, blobCy);;
		// 延迟100ms
		SysTick_DelayMs(100);
	}
}
