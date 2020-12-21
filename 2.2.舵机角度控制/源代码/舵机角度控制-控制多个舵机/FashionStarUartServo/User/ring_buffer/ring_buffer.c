#include "ring_buffer.h"

// 队列重置
void RingBuffer_Reset(RingBufferTypeDef *ringBuf){
	// 队首指针跟队尾指针都指向buffer的首个地址
	ringBuf->head = 0;
	ringBuf->tail = 0;
}

// 创建队列
void RingBuffer_Init(RingBufferTypeDef* ringBuf, uint16_t capacity, uint8_t *buffer){
    // RingBufferTypeDef *ringBuf = (RingBufferTypeDef *)malloc(sizeof(RingBufferTypeDef));
	// Buffer内存申请成功
	ringBuf->buffer = buffer;// (uint8_t *)malloc(ringBuf->size);
	ringBuf->head = 0;
	ringBuf->tail = 0;
	// 尺寸size需要比容量capacity大一
	ringBuf->size = capacity + 1;
	// 数值初始化置为0
	for (uint16_t i=0; i<ringBuf->size; i++){
		ringBuf->buffer[i] = 0;
	}
	// printf("[RingBuffer_Init] ring buffer init pbuffer=%p\n", ringBuf->buffer);
}

// 销毁队列
//void RingBuffer_Destory(RingBufferTypeDef *ringBuf){
//	free(ringBuf->buffer);
//	free(ringBuf);
//}

// 获得环形队列的容量
uint16_t RingBuffer_GetCapacity(RingBufferTypeDef *ringBuf){
	return ringBuf->size - 1;
}

// 获取环形队列已经存好的字节数
uint16_t RingBuffer_GetByteUsed(RingBufferTypeDef *ringBuf){
	if (ringBuf->head > ringBuf->tail){
		return RingBuffer_GetCapacity(ringBuf) - (ringBuf->head - ringBuf->tail - 1);
	}else{
		return ringBuf->tail - ringBuf->head;
	}
}

// 获取队列的剩余容量
uint16_t RingBuffer_GetByteFree(RingBufferTypeDef *ringBuf){
	return RingBuffer_GetCapacity(ringBuf) - RingBuffer_GetByteUsed(ringBuf);
}

// 判断环形队列是否为空
uint8_t RingBuffer_IsEmpty(RingBufferTypeDef *ringBuf){
	return RingBuffer_GetByteFree(ringBuf) == RingBuffer_GetCapacity(ringBuf);
}

// 判断环形队列是否已满
uint8_t RingBuffer_IsFull(RingBufferTypeDef *ringBuf){
	return RingBuffer_GetByteFree(ringBuf) == 0;
}

// 根据索引号读取第i个元素
uint8_t RingBuffer_GetValueByIndex(RingBufferTypeDef *ringBuf, uint16_t index){
	if (index >= RingBuffer_GetByteUsed(ringBuf)){
		// 索引号超出了范围
		return 0;
	}
	uint16_t rbIdx = (ringBuf->head + index + 1) % ringBuf->size;
	return ringBuf->buffer[rbIdx];
}

// 弹出队首元素
uint8_t RingBuffer_Pop(RingBufferTypeDef *ringBuf){
	uint8_t temp;
	if(RingBuffer_IsEmpty(ringBuf)){
		return 0;
	}
	else{
		ringBuf->head =(ringBuf->head + 1)%ringBuf->size;
		temp = ringBuf->buffer[ringBuf->head];
		ringBuf->buffer[ringBuf->head] = 0;
		return temp;
	}
	
}


// 写入队尾元素
void RingBuffer_Push(RingBufferTypeDef *ringBuf, uint8_t value){
	//printf("[RingBuffer_Push] before detect isFull\n");
	if (RingBuffer_IsFull(ringBuf)){
		// 队列已经写满了, 只能先弹出队首的元素
		RingBuffer_Pop(ringBuf);
	}
	ringBuf->tail =(ringBuf->tail + 1)%ringBuf->size;
	//printf("[RingBuffer_Push]  ringBuf tail index: %d\n", ringBuf->tail);
	ringBuf->buffer[ringBuf->tail] = value;
	//printf("[RingBuffer_Push] tail %d\n", ringBuf->buffer[ringBuf->tail]);

}

// 读取单个字节
uint8_t RingBuffer_ReadByte(RingBufferTypeDef *ringBuf){
	return RingBuffer_Pop(ringBuf);
}

// 读取字节数组
void RingBuffer_ReadByteArray(RingBufferTypeDef *ringBuf, uint8_t* dest, uint16_t size){
	for(uint16_t idx=0; idx<size; idx++){
		dest[idx] = RingBuffer_Pop(ringBuf);
	}
}

// 读取有符号Short整数(两个字节)
int16_t RingBuffer_ReadShort(RingBufferTypeDef *ringBuf){
	int16_t value;
	uint8_t *p = (uint8_t *)&value;
	RingBuffer_ReadByteArray(ringBuf, p, 2);
	return value;
}

// 读取无符号Short整数(两个字节)
uint16_t RingBuffer_ReadUShort(RingBufferTypeDef *ringBuf){
	uint16_t value;
	uint8_t *p = (uint8_t *)&value;
	RingBuffer_ReadByteArray(ringBuf, p, 2);
	return value;
}

// 读取有符号Long类型的整数(四个字节)
int32_t RingBuffer_ReadLong(RingBufferTypeDef *ringBuf){
	int32_t value;
	uint8_t *p = (uint8_t *)&value;
	RingBuffer_ReadByteArray(ringBuf, p, 4);
	return value;
}

// 读取无符号Long类型的整数(四个字节)
uint32_t RingBuffer_ReadULong(RingBufferTypeDef *ringBuf){
	uint32_t value;
	uint8_t *p = (uint8_t *)&value;
	RingBuffer_ReadByteArray(ringBuf, p, 4);
	return value;
}

// 读取浮点数(四个字节)
float RingBuffer_ReadFloat(RingBufferTypeDef *ringBuf){
	float value;
	uint8_t *p = (uint8_t *)&value;
	RingBuffer_ReadByteArray(ringBuf, p, 4);
	return value;
}

// 写入单个字节
void RingBuffer_WriteByte(RingBufferTypeDef *ringBuf, uint8_t value){
	return RingBuffer_Push(ringBuf, value);
}

// 写入字节数组
void RingBuffer_WriteByteArray(RingBufferTypeDef *ringBuf, uint8_t* src, uint16_t size){
	for(uint16_t idx=0; idx<size; idx++){
		RingBuffer_Push(ringBuf, src[idx]);
	}
}
// 写入有符号Short整数(两个字节)
void RingBuffer_WriteShort(RingBufferTypeDef *ringBuf, int16_t value){
	uint8_t *p= (uint8_t *)&value;
	RingBuffer_WriteByteArray(ringBuf, p, 2);
}

// 写入无符号Short整数(两个字节)
void RingBuffer_WriteUShort(RingBufferTypeDef *ringBuf, uint16_t value){
	uint8_t *p= (uint8_t *)&value;
	RingBuffer_WriteByteArray(ringBuf, p, 2);
}

// 写入有符号Long类型的整数(四个字节)
void RingBuffer_WriteLong(RingBufferTypeDef *ringBuf, int32_t value){
	uint8_t *p= (uint8_t *)&value;
	RingBuffer_WriteByteArray(ringBuf, p, 4);
}

// 写入无符号Long类型的整数(四个字节)
void RingBuffer_WriteULong(RingBufferTypeDef *ringBuf, uint32_t value){
	uint8_t *p= (uint8_t *)&value;
	RingBuffer_WriteByteArray(ringBuf, p, 4);
}

// 写入浮点数(四个字节)
void RingBuffer_WriteFloat(RingBufferTypeDef *ringBuf, float value){
	uint8_t *p= (uint8_t *)&value;
	RingBuffer_WriteByteArray(ringBuf, p, 4);
}


// 计算校验和
uint8_t RingBuffer_GetChecksum(RingBufferTypeDef *ringBuf){
	uint16_t nByte = RingBuffer_GetByteUsed(ringBuf);
	uint32_t bSum=0;
	// printf("[RingBuffer_GetChecksum] \r\n");
	// 设置指针初始化位置
	
	for(int i=0; i < nByte; i++){
		// 指针逐次右移
		bSum = (bSum + RingBuffer_GetValueByIndex(ringBuf, i))% 256;
	}
	//计算校验和
	return bSum;
}
