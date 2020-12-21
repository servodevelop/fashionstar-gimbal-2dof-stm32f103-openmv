/*
 * Fashion Star 串口舵机驱动库
 * Version: v0.0.1
 * Author: Kyle
 * UpdateTime: 2019/10/23
 */
#include "fashion_star_uart_servo.h"

// 数据帧转换为字节数组
void FSUS_Package2RingBuffer(PackageTypeDef *pkg,  RingBufferTypeDef *ringBuf){
    uint8_t checksum; // 校验和
    // 写入帧头
    RingBuffer_WriteUShort(ringBuf, pkg->header);
    // 写入指令ID
    RingBuffer_WriteByte(ringBuf, pkg->cmdId);
    // 写入包的长度
    RingBuffer_WriteByte(ringBuf, pkg->size);
    // 写入内容主题
    RingBuffer_WriteByteArray(ringBuf, pkg->content, pkg->size);
    // 计算校验和
    checksum = RingBuffer_GetChecksum(ringBuf);
    // 写入校验和
    RingBuffer_WriteByte(ringBuf, checksum);

}

// 计算Package的校验和
uint8_t FSUS_CalcChecksum(PackageTypeDef *pkg){
    uint8_t checksum;
	// 初始化环形队列
	RingBufferTypeDef ringBuf;
	uint8_t pkgBuf[FSUS_PACK_RESPONSE_MAX_SIZE+1];
	RingBuffer_Init(&ringBuf, FSUS_PACK_RESPONSE_MAX_SIZE, pkgBuf);
    // 将Package转换为ringbuffer
	// 在转换的时候,会自动的计算checksum
    FSUS_Package2RingBuffer(pkg, &ringBuf);
	// 获取环形队列队尾的元素(即校验和的位置)
	checksum = RingBuffer_GetValueByIndex(&ringBuf, RingBuffer_GetByteUsed(&ringBuf)-1);
    return checksum;
}

// 判断是否为有效的请求头的
FSUS_STATUS FSUS_IsValidResponsePackage(PackageTypeDef *pkg){
    // 帧头数据不对
    if (pkg->header != FSUS_PACK_RESPONSE_HEADER){
        // 帧头不对
        return FSUS_STATUS_WRONG_RESPONSE_HEADER;
    }
    // 判断控制指令是否有效 指令范围超出
    if (pkg->cmdId > 10){
        return FSUS_STATUS_UNKOWN_CMD_ID;
    }
    // 参数的size大于FSUS_PACK_RESPONSE_MAX_SIZE里面的限制
    if (pkg->size > (FSUS_PACK_RESPONSE_MAX_SIZE - 5)){
        return FSUS_STATUS_SIZE_TOO_BIG;
    }
    // 校验和不匹配
    if (FSUS_CalcChecksum(pkg) != pkg->checksum){
        return FSUS_STATUS_CHECKSUM_ERROR;
    }
    // 数据有效
    return FSUS_STATUS_SUCCESS;
}

// 字节数组转换为数据帧
FSUS_STATUS FSUS_RingBuffer2Package(RingBufferTypeDef *ringBuf, PackageTypeDef *pkg){
    // 申请内存
    pkg = (PackageTypeDef *)malloc(sizeof(PackageTypeDef));
    // 读取帧头
    pkg->header = RingBuffer_ReadUShort(ringBuf);
    // 读取指令ID
    pkg->cmdId = RingBuffer_ReadByte(ringBuf);
    // 读取包的长度
    pkg->size = RingBuffer_ReadByte(ringBuf);
    // 申请参数的内存空间
    // pkg->content = (uint8_t *)malloc(pkg->size);
    // 写入content
    RingBuffer_ReadByteArray(ringBuf, pkg->content, pkg->size);
    // 写入校验和
    pkg->checksum = RingBuffer_ReadByte(ringBuf);
    // 返回当前的数据帧是否为有效反馈数据帧
    return FSUS_IsValidResponsePackage(pkg);
}

// 构造发送数据帧
void FSUS_SendPackage(Usart_DataTypeDef *usart, uint8_t cmdId, uint8_t size, uint8_t *content){
    // 申请内存
	// printf("[Package] malloc for pkg\r\n");
    PackageTypeDef pkg;
	
    // 设置帧头
    pkg.header = FSUS_PACK_REQUEST_HEADER;
    // 设置指令ID
    pkg.cmdId = cmdId;
    // 设置尺寸
    pkg.size = size;
	// 逐一拷贝数组里面的内容
	for(int i=0; i<size; i++){
		pkg.content[i] = content[i];
	}
    // 将pkg发送到发送缓冲区sendBuf里面
    FSUS_Package2RingBuffer(&pkg, usart->sendBuf);
	// 通过串口将数据发送出去
    Usart_SendAll(usart);
}

// 接收数据帧 (在接收的时候动态的申请内存)
FSUS_STATUS FSUS_RecvPackage(Usart_DataTypeDef *usart, PackageTypeDef *pkg){
	
	pkg->status = 0; // Package状态初始化
	
    uint8_t bIdx = 0; // 接收的参数字节索引号
    uint16_t header = 0; // 帧头
	
    // 倒计时开始
    SysTick_CountdownBegin(FSUS_TIMEOUT_MS);

    // 如果没有超时
    while (!SysTick_CountdownIsTimeout()){
        if (RingBuffer_GetByteUsed(usart->recvBuf) == 0){
			// 没有新的字节读进来, 继续等待
            continue;
        }

        // 查看校验码是否已经接收到
        if(pkg->status & FSUS_RECV_FLAG_CONTENT){
            // 参数内容接收完成
            // 接收校验码
            pkg->checksum = RingBuffer_ReadByte(usart->recvBuf);
            // 设置状态码-校验码已经接收
            pkg->status = pkg->status | FSUS_RECV_FLAG_CHECKSUM;
            // 直接进行校验码检查
            if (FSUS_CalcChecksum(pkg) != pkg->checksum){
                // 撤销倒计时
                SysTick_CountdownCancel();
                // 校验码错误
                return FSUS_STATUS_CHECKSUM_ERROR;
            }else{
                // 撤销倒计时
                SysTick_CountdownCancel();
                // 数据帧接收成功
                return FSUS_STATUS_SUCCESS;
            }
        }else if(pkg->status & FSUS_RECV_FLAG_SIZE){
            // Size已经接收完成
            // 接收参数字节
            pkg->content[bIdx] = RingBuffer_ReadByte(usart->recvBuf);
            bIdx ++;
            // 判断是否接收完成
            if (bIdx == pkg->size){
                // 标记为参数接收完成
                pkg->status = pkg->status | FSUS_RECV_FLAG_CONTENT;
            }
        }else if(pkg->status & FSUS_RECV_FLAG_CMD_ID){
            // 指令接收完成
            // 接收尺寸信息
            pkg->size = RingBuffer_ReadByte(usart->recvBuf);
            // 判断长度size是否合法
            // 参数的size大于FSUS_PACK_RESPONSE_MAX_SIZE里面的限制
            if (pkg->size > (FSUS_PACK_RESPONSE_MAX_SIZE - 5)){
                // 撤销倒计时
                SysTick_CountdownCancel();
                return FSUS_STATUS_SIZE_TOO_BIG;
            }
            // 设置尺寸接收完成的标志位
            pkg->status = pkg->status | FSUS_RECV_FLAG_SIZE;
        }else if(pkg->status & FSUS_RECV_FLAG_HEADER){
            // 帧头已接收 
            // 接收指令ID
            pkg->cmdId = RingBuffer_ReadByte(usart->recvBuf);
            // 判断指令是否合法
            // 判断控制指令是否有效 指令范围超出
            if (pkg->cmdId > 10){
                // 撤销倒计时
                SysTick_CountdownCancel();
                return FSUS_STATUS_UNKOWN_CMD_ID;
            }
            // 设置cmdId已经接收到标志位
            pkg->status = pkg->status | FSUS_RECV_FLAG_CMD_ID;
        }else{
            // 接收帧头
            if (header == 0){
                // 接收第一个字节
                header = RingBuffer_ReadByte(usart->recvBuf);
                // 判断接收的第一个字节对不对
                if (header != (FSUS_PACK_RESPONSE_HEADER&0x0F)){
                    // 第一个字节不对 header重置为0
                    header = 0;
                }
            }else if(header == (FSUS_PACK_RESPONSE_HEADER&0x0F)){
                // 接收帧头第二个字节
                header =  header | (RingBuffer_ReadByte(usart->recvBuf) << 8);
				// 检查第二个字节是否正确
                if(header != FSUS_PACK_RESPONSE_HEADER){
                    header = 0;
                }else{
                    pkg->header = header;
                    // 帧头接收成功
                    pkg->status = pkg->status | FSUS_RECV_FLAG_HEADER; 
                }
            }else{
                header = 0;
            }
        }
    }
    // 等待超时
    return FSUS_STATUS_TIMEOUT;
}

// 舵机通讯检测
// 注: 如果没有舵机响应这个Ping指令的话, 就会超时
FSUS_STATUS FSUS_Ping(Usart_DataTypeDef *usart, uint8_t servoId){
	uint8_t statusCode; // 状态码
	uint8_t ehcoServoId; // PING得到的舵机ID
	// printf("[PING]Send Ping Package\r\n");
	// 发送请求包
	FSUS_SendPackage(usart, FSUS_CMD_PING, 1, &servoId);
	// 接收返回的Ping
	PackageTypeDef pkg;
	statusCode = FSUS_RecvPackage(usart, &pkg);
	if(statusCode == FSUS_STATUS_SUCCESS){
		// 进一步检查ID号是否匹配
		ehcoServoId = (uint8_t)pkg.content[0];
		if (ehcoServoId != servoId){
			// 反馈得到的舵机ID号不匹配
			return FSUS_STATUS_ID_NOT_MATCH;
		}
	}
	return statusCode;
}

// 重置舵机的用户资料
FSUS_STATUS FSUS_ResetUserData(Usart_DataTypeDef *usart, uint8_t servoId){
	const uint8_t size = 1;
	FSUS_STATUS statusCode;
	// 发送请求包
	FSUS_SendPackage(usart, FSUS_CMD_RESET_USER_DATA, size, &servoId);
	// 接收重置结果
	PackageTypeDef pkg;
	statusCode = FSUS_RecvPackage(usart, &pkg);
	if (statusCode == FSUS_STATUS_SUCCESS){
		// 成功的接收到反馈数据
		// 读取反馈数据中的result
		uint8_t result = (uint8_t)pkg.content[1];
		if (result == 1){
			return FSUS_STATUS_SUCCESS;
		}else{
			return FSUS_STATUS_FAIL;
		}
	}
	return statusCode;
}

// 读取数据
FSUS_STATUS FSUS_ReadData(Usart_DataTypeDef *usart, uint8_t servoId,  uint8_t address, uint8_t *value, uint8_t *size){
	FSUS_STATUS statusCode;
	// 构造content
	uint8_t buffer[2] = {servoId, address};
	// 发送请求数据
	FSUS_SendPackage(usart, FSUS_CMD_READ_DATA, 2, buffer);
	// 接收返回信息
	PackageTypeDef pkg;
	statusCode = FSUS_RecvPackage(usart, &pkg);
	if (statusCode == FSUS_STATUS_SUCCESS){
		// 读取数据
		// 读取数据是多少个位
		*size = pkg.size - 2; // content的长度减去servoId跟address的长度
		// 数据拷贝
		for (int i=0; i<*size; i++){
			value[i] = pkg.content[i+2];
		}
	}
	return statusCode;
}

// 写入数据
FSUS_STATUS FSUS_WriteData(Usart_DataTypeDef *usart, uint8_t servoId, uint8_t address, uint8_t *value, uint8_t size){
	FSUS_STATUS statusCode;
	// 构造content
	uint8_t buffer[size+2]; // 舵机ID + 地址位Address + 数据byte数
	buffer[0] = servoId;
	buffer[1] = address;
	// 拷贝数据
	for (int i=0; i<size; i++){
		buffer[i+2] = value[i];
	}
	// 发送请求数据
	FSUS_SendPackage(usart, FSUS_CMD_WRITE_DATA, size+2, buffer);
	// 接收返回信息
	PackageTypeDef pkg;
	statusCode = FSUS_RecvPackage(usart, &pkg);
	if (statusCode == FSUS_STATUS_SUCCESS){
		uint8_t result = pkg.content[2];
		if(result == 1){
			statusCode = FSUS_STATUS_SUCCESS;
		}else{
			statusCode = FSUS_STATUS_FAIL;
		}
	}
	return statusCode;
}

// 读取单个舵机所有的资料信息(注: 未实现) 
// 写入单个舵机所有的资料信息(注: 未实现)
// 批次写入过于麻烦, 用户手动配置单个舵机就得写至少32行参数设置代码
// 而且不同的控制模式, 有些是不需要设置的.
// 推荐单项设置

/* 
 * 轮式控制模式
 * speed单位 °/s
 */
FSUS_STATUS FSUS_WheelMove(Usart_DataTypeDef *usart, uint8_t servoId, uint8_t method, uint16_t speed, uint16_t value){
	// 创建环形缓冲队列
	const uint8_t size = 6;
	uint8_t buffer[size+1];
	RingBufferTypeDef ringBuf;
	RingBuffer_Init(&ringBuf, size, buffer);
	// 写入content
	RingBuffer_WriteByte(&ringBuf, servoId);  // 舵机ID
	RingBuffer_WriteByte(&ringBuf, method);   // 写入执行方式与旋转方向
	RingBuffer_WriteUShort(&ringBuf, speed);  // 设置舵机的旋转速度 °/s
	RingBuffer_WriteUShort(&ringBuf, value);
	// 发送请求包
	FSUS_SendPackage(usart, FSUS_CMD_SPIN, size, buffer+1);
	
	return FSUS_STATUS_SUCCESS;
}

// 轮式模式, 舵机停止转动
FSUS_STATUS FSUS_WheelStop(Usart_DataTypeDef *usart, uint8_t servoId){
	uint8_t method = 0x00;
	uint16_t speed = 0;
	uint16_t value = 0;
	return FSUS_WheelMove(usart, servoId, method, speed, value);
}

// 轮式模式 不停的旋转
FSUS_STATUS FSUS_WheelKeepMove(Usart_DataTypeDef *usart, uint8_t servoId, uint8_t is_cw, uint16_t speed){
	uint8_t method = 0x01; // 持续旋转
	if (is_cw){
		// 顺时针旋转
		method = method | 0x80;
	}
	uint16_t value = 0;
	return FSUS_WheelMove(usart, servoId, method, speed, value);
}

// 轮式模式 按照特定的速度旋转特定的时间
FSUS_STATUS FSUS_WheelMoveTime(Usart_DataTypeDef *usart, uint8_t servoId, uint8_t is_cw, uint16_t speed, uint16_t nTime){
	uint8_t method = 0x03; // 旋转一段时间
	if (is_cw){
		// 顺时针旋转
		method = method | 0x80;
	}
	return FSUS_WheelMove(usart, servoId, method, speed, nTime);
}

// 轮式模式 旋转特定的圈数
FSUS_STATUS FSUS_WheelMoveNCircle(Usart_DataTypeDef *usart, uint8_t servoId, uint8_t is_cw, uint16_t speed, uint16_t nCircle){
	uint8_t method = 0x02; // 旋转特定的圈数
	if (is_cw){
		// 顺时针旋转
		method = method | 0x80;
	}
	
	return FSUS_WheelMove(usart, servoId, method, speed, nCircle);
}

// 设置舵机的角度
// @angle 单位度
// @interval 单位ms
// @power 舵机执行功率 单位mW
//        若power=0或者大于保护值
FSUS_STATUS FSUS_SetServoAngle(Usart_DataTypeDef *usart, uint8_t servoId, float angle, uint16_t interval, uint16_t power, uint8_t wait){
	// 创建环形缓冲队列
	const uint8_t size = 7;
	uint8_t buffer[size+1];
	RingBufferTypeDef ringBuf;
	RingBuffer_Init(&ringBuf, size, buffer);	
	// 构造content
	RingBuffer_WriteByte(&ringBuf, servoId);
	RingBuffer_WriteShort(&ringBuf, (int16_t)(10*angle));
	RingBuffer_WriteUShort(&ringBuf, interval);
	RingBuffer_WriteUShort(&ringBuf, power);
	// 发送请求包
	// 注: 因为用的是环形队列 head是空出来的,所以指针需要向后推移一个字节
	FSUS_SendPackage(usart, FSUS_CMD_ROTATE, size, buffer+1);
	
	if (wait){
		// 阻塞式
		SysTick_DelayMs(interval);
	}
	
	return FSUS_STATUS_SUCCESS;
}

// 查询单个舵机的角度信息 angle 单位度
FSUS_STATUS FSUS_QueryServoAngle(Usart_DataTypeDef *usart, uint8_t servoId, float *angle){
	const uint8_t size = 1; // 请求包content的长度
	uint8_t ehcoServoId;
	int16_t echoAngle;
	
	// 发送舵机角度请求包
	FSUS_SendPackage(usart, FSUS_CMD_READ_ANGLE, size, &servoId);
	// 接收返回的Ping
	PackageTypeDef pkg;
	uint8_t statusCode = FSUS_RecvPackage(usart, &pkg);
	if (statusCode == FSUS_STATUS_SUCCESS){
		// 成功的获取到舵机角度回读数据
		ehcoServoId = (uint8_t)pkg.content[0];
		// 检测舵机ID是否匹配
		if (ehcoServoId != servoId){
			// 反馈得到的舵机ID号不匹配
			return FSUS_STATUS_ID_NOT_MATCH;
		}
		
		// 提取舵机角度
		echoAngle = (int16_t)(pkg.content[1] | (pkg.content[2] << 8));
		*angle = (float)(echoAngle / 10.0);
	}
    return statusCode;
}	

// 舵机阻尼模式
FSUS_STATUS FSUS_DampingMode(Usart_DataTypeDef *usart, uint8_t servoId, uint16_t power){
	const uint8_t size = 3; // 请求包content的长度
	uint8_t buffer[size+1]; // content缓冲区
	RingBufferTypeDef ringBuf; // 创建环形缓冲队列
	RingBuffer_Init(&ringBuf, size, buffer); // 缓冲队列初始化
	// 构造content
	RingBuffer_WriteByte(&ringBuf, servoId);
	RingBuffer_WriteUShort(&ringBuf, power);
	// 发送请求包
	// 注: 因为用的是环形队列 head是空出来的,所以指针需要向后推移一个字节
	FSUS_SendPackage(usart, FSUS_CMD_DAMPING, size, buffer+1);
	return FSUS_STATUS_SUCCESS;
}
