#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "stm32f10x.h"
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

// 环形队列结构体
typedef struct
{  
    uint8_t *buffer; // 缓冲区
    uint16_t head; // 队首指针
    uint16_t tail; // 队尾指针
    uint16_t size; // 环形队列的尺寸
} RingBufferTypeDef;

// 创建队列
void RingBuffer_Init(RingBufferTypeDef* ringBuf, uint16_t capacity, uint8_t *buffer);
// 队列重置
void RingBuffer_Reset(RingBufferTypeDef *ringBuf);
// 销毁队列
void RingBuffer_Destory(RingBufferTypeDef *ringBuf);
// 获取队列的容量
uint16_t RingBuffer_GetCapacity(RingBufferTypeDef *ringBuf);
// 获取环形队列已经存好的字节数
uint16_t RingBuffer_GetByteUsed(RingBufferTypeDef *ringBuf);
// 获取队列的剩余的空闲字节
uint16_t RingBuffer_GetByteFree(RingBufferTypeDef *ringBuf);
// 队列是否为空
uint8_t RingBuffer_IsEmpty(RingBufferTypeDef *ringBuf);
// 队列是否已满
uint8_t RingBuffer_IsFull(RingBufferTypeDef *ringBuf);
// 根据索引号读取第i个元素
uint8_t RingBuffer_GetValueByIndex(RingBufferTypeDef *ringBuf, uint16_t index);
// 弹出队首元素
uint8_t RingBuffer_Pop(RingBufferTypeDef *ringBuf);
// 读取单个字节
uint8_t RingBuffer_ReadByte(RingBufferTypeDef *ringBuf);
// 读取字节数组
void RingBuffer_ReadByteArray(RingBufferTypeDef *ringBuf, uint8_t* dest, uint16_t size);
// 读取有符号Short整数(两个字节)
int16_t RingBuffer_ReadShort(RingBufferTypeDef *ringBuf);
// 读取无符号Short整数(两个字节)
uint16_t RingBuffer_ReadUShort(RingBufferTypeDef *ringBuf);
// 读取有符号Long类型的整数(四个字节)
int32_t RingBuffer_ReadLong(RingBufferTypeDef *ringBuf);
// 读取无符号Long类型的整数(四个字节)
uint32_t RingBuffer_ReadULong(RingBufferTypeDef *ringBuf);
// 读取浮点数(四个字节)
float RingBuffer_ReadFloat(RingBufferTypeDef *ringBuf);
// 写入队尾元素
void RingBuffer_Push(RingBufferTypeDef *ringBuf, uint8_t value);
// 写入单个字节
void RingBuffer_WriteByte(RingBufferTypeDef *ringBuf, uint8_t value);
// 写入字节数组
void RingBuffer_WriteByteArray(RingBufferTypeDef *ringBuf, uint8_t* src, uint16_t size);
// 写入有符号Short整数(两个字节)
void RingBuffer_WriteShort(RingBufferTypeDef *ringBuf, int16_t value);
// 写入无符号Short整数(两个字节)
void RingBuffer_WriteUShort(RingBufferTypeDef *ringBuf, uint16_t value);
// 写入有符号Long类型的整数(四个字节)
void RingBuffer_WriteLong(RingBufferTypeDef *ringBuf, int32_t value);
// 写入无符号Long类型的整数(四个字节)
void RingBuffer_WriteULong(RingBufferTypeDef *ringBuf, uint32_t value);
// 写入浮点数(四个字节)
void RingBuffer_WriteFloat(RingBufferTypeDef *ringBuf, float value);
// 计算缓冲区所有自己的校验和()
uint8_t RingBuffer_GetChecksum(RingBufferTypeDef *ringBuf);

#endif

