#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "ring_buffer.h"

#define USART_RECV_BUF_SIZE 500
#define USART_SEND_BUF_SIZE 500

// UART Enable
#define USART1_ENABLE 1
#define USART2_ENABLE 1
#define USART3_ENABLE 0


// 串口1相关参数
#if USART1_ENABLE
    // 波特率
    #define USART1_BAUDRATE 115200
    // 时钟定义
    #define USART1_CLK RCC_APB2Periph_USART1
    #define USART1_APBxClkCmd RCC_APB2PeriphClockCmd
    #define USART1_GPIO_CLK RCC_APB2Periph_GPIOA
    #define USART1_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd
    // GPIO引脚定义
    #define USART1_TX_GPIO_PORT GPIOA
    #define USART1_TX_GPIO_PIN GPIO_Pin_9
    #define USART1_RX_GPIO_PORT GPIOA
    #define USART1_RX_GPIO_PIN  GPIO_Pin_10
#endif

// 串口2相关参数
#if USART2_ENABLE
    #define USART2_BAUDRATE 115200
    // 时钟定义
    #define USART2_CLK RCC_APB1Periph_USART2
    #define USART2_APBxClkCmd RCC_APB1PeriphClockCmd
    #define USART2_GPIO_CLK RCC_APB2Periph_GPIOA
    #define USART2_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd
    // GPIO引脚定义
    #define USART2_TX_GPIO_PORT GPIOA
    #define USART2_TX_GPIO_PIN GPIO_Pin_2
    #define USART2_RX_GPIO_PORT GPIOA
    #define USART2_RX_GPIO_PIN  GPIO_Pin_3
#endif

// 串口3相关参数
#if USART3_ENABLE
    #define USART3_BAUDRATE 115200
    // 时钟定义
    #define USART3_CLK RCC_APB1Periph_USART3
    #define USART3_APBxClkCmd RCC_APB1PeriphClockCmd
    #define USART3_GPIO_CLK RCC_APB2Periph_GPIOB
    #define USART3_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd
    // GPIO引脚定义
    #define USART3_TX_GPIO_PORT GPIOB
    #define USART3_TX_GPIO_PIN GPIO_Pin_10
    #define USART3_RX_GPIO_PORT GPIOB
    #define USART3_RX_GPIO_PIN  GPIO_Pin_11
#endif

typedef struct
{  
    USART_TypeDef *pUSARTx;
    // 发送端缓冲区
    RingBufferTypeDef *sendBuf;
	// 接收端缓冲区
    RingBufferTypeDef *recvBuf;
} Usart_DataTypeDef;

//memset(&gc_block, 0, sizeof(parser_block_t))
#if USART1_ENABLE
    extern Usart_DataTypeDef usart1;
    extern uint8_t usart1SendBuf[USART_SEND_BUF_SIZE+1];
	extern uint8_t usart1RecvBuf[USART_RECV_BUF_SIZE+1];
	extern RingBufferTypeDef usart1SendRingBuf;
	extern RingBufferTypeDef usart1RecvRingBuf;
#endif

#if USART2_ENABLE
	extern uint8_t usart2SendBuf[USART_SEND_BUF_SIZE+1];
	extern uint8_t usart2RecvBuf[USART_RECV_BUF_SIZE+1];
	extern RingBufferTypeDef usart2SendRingBuf;
	extern RingBufferTypeDef usart2RecvRingBuf;
	extern Usart_DataTypeDef usart2;

#endif

#if USART3_ENABLE
    extern uint8_t usart3SendBuf[USART_SEND_BUF_SIZE+1];
	extern uint8_t usart3RecvBuf[USART_RECV_BUF_SIZE+1];
	extern RingBufferTypeDef usart3SendRingBuf;
	extern RingBufferTypeDef usart3RecvRingBuf;
	extern Usart_DataTypeDef usart3;
#endif

// 配置串口
void Usart_Init(void);
// 发送字节
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch);
// 发送字节数组
void Usart_SendByteArr(USART_TypeDef *pUSARTx, uint8_t *byteArr, uint16_t size);
// 发送字符串
void Usart_SendString(USART_TypeDef *pUSARTx, char *str);
// 将串口发送缓冲区的内容全部发出去
void Usart_SendAll(Usart_DataTypeDef *usart);
#endif
